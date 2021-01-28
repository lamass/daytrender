#include "client.h"

#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>

#define GET_ERROR_FUNC			"get_error"
#define MAX_CANDLES_FUNC		"max_candles"
#define INIT_FUNC				"init"
#define GET_CANDLES_FUNC		"get_candles"
#define MARKET_ORDER_FUNC		"marker_order"
#define TO_INTERVAL_FUNC		"to_interval"
#define GET_ACCT_INFO_FUNC		"get_account_info"
#define PAPER_MINIMUM_FUNC		"paper_minimum"
#define PAPER_FEE_FUNC			"paper_fee"
#define BACKTEST_INTERVALS_FUNC	"backtest_intervals"

namespace daytrender
{
	Client::Client(const std::string& label, const std::string& filepath,
		const std::vector<std::string>& credentials, double risk, double max_loss,
		double history_length, int leverage)
	{
		_label = label;
		_risk = risk;
		_filename = hirzel::str::get_filename(filepath);
		_max_loss = max_loss;
		_history_length = history_length;
		_leverage = leverage;

		_handle = new hirzel::Plugin(filepath);

		if (!_handle->is_lib_bound())
		{
			errorf("%s failed to bind: %s", _filename, _handle->get_error());
			return;
		}

		_init = (bool(*)(const std::vector<std::string>&))_handle->bind_function(INIT_FUNC);
		if (!_init) flag_error();

		_get_candles = (bool(*)(CandleSet&, const std::string&))_handle->bind_function(GET_CANDLES_FUNC);
		if (!_get_candles) flag_error();

		_get_account_info = (bool(*)(AccountInfo&))_handle->bind_function(GET_ACCT_INFO_FUNC);
		if (!_get_account_info) flag_error();

		_market_order = (bool(*)(const std::string&, double))_handle->bind_function("market_order");
		if (!_market_order) flag_error();

		_get_shares = (bool(*)(double&, const std::string&))_handle->bind_function("get_shares");
		if (!_get_shares) flag_error();

		_close_all_positions = (bool(*)())_handle->bind_function("close_all_positions");
		if (!_close_all_positions) flag_error();

		_market_open = (bool(*)(bool&))_handle->bind_function("market_open");
		if (!_market_open) flag_error();

		_get_price = (bool(*)(double&, const std::string&))_handle->bind_function("get_price");
		if (!_get_price) flag_error();

		_set_leverage = (bool(*)(int))_handle->bind_function("set_leverage");
		if (!_set_leverage) flag_error();

		// getters

		_to_interval = (bool(*)(const char*, int))_handle->bind_function(TO_INTERVAL_FUNC);
		if (!_to_interval) flag_error();

		_max_candles = (int(*)())_handle->bind_function(MAX_CANDLES_FUNC);
		if (!_max_candles) flag_error();

		_paper_fee = (double(*)())_handle->bind_function(PAPER_FEE_FUNC);
		if (!_paper_fee) flag_error();

		_paper_minimum = (double(*)())_handle->bind_function(PAPER_MINIMUM_FUNC);
		if (!_paper_minimum) flag_error();

		_backtest_intervals = (void(*)(std::vector<int>&))_handle->bind_function(BACKTEST_INTERVALS_FUNC);
		if (!_backtest_intervals) flag_error();

		_get_error = (void(*)(std::string&))_handle->bind_function(GET_ERROR_FUNC);
		if (!_get_error) flag_error();

		_bound = _handle->all_bound();

		if (_bound)
		{

			if (!_init(credentials))
			{
				flag_error();
			}
			else
			{
				_live = true;
			}
			
			if (!set_leverage(_leverage))
			{
				_live = false;
			}

			if (_live)
			{
				successf("Successfully loaded %s client: '%s'", _label, _filename);
			}
			else
			{
				errorf("Failed initialize %s client: '%s'", _label, _filename);
			}
		}
		else
		{
			errorf("Failed to bind to all functions of: '%s': %s", _filename, _handle->get_error());
		}
	}

	Client::~Client()
	{
		delete _handle;
	}

	bool Client::func_ok(const char* label, void(*func)()) const
	{
		if (!_live)
		{
			errorf("%s: '%s': client is not live.", _filename, label);
			return false;
		}

		if (func == nullptr)
		{
			errorf("%s: %s is not bound and cannot be executed!", _filename, label);
			return false;
		}

		return true;
	}

	void Client::flag_error() const
	{
		errorf("%s: %s", _filename, get_error());
		_live = false;
	}

	CandleSet Client::get_candles(const std::string& ticker, int interval, int max) const
	{
		if (!func_ok(GET_CANDLES_FUNC, (void(*)())_get_candles)) return CandleSet();

		if (max == 0)
		{
			max = max_candles();
		}
		else if (max > max_candles())
		{
			errorf("%s: requested more candles than maximum!", _filename);
			return {};
		}
		else if (max <= 0)
		{
			errorf("%s: requested negative amount of candles!", _filename);
			return {};
		}

		CandleSet candles(max, interval);
		bool res = _get_candles(candles, ticker);
		if (!res) flag_error();

		return candles;
	}

	AccountInfo Client::get_account_info()
	{
		if (!func_ok("get_account_info", (void(*)())_get_account_info)) return AccountInfo();

		AccountInfo info;
		bool res = _get_account_info(info);
		if (!res) flag_error();

		// handling equity_history and loss management
		long long curr_time = hirzel::sys::get_seconds();
		_equity_history.push_back({ curr_time, info.equity() });

		while (curr_time - _equity_history[0].first > (long long)(_history_length * 3600))
		{
			_equity_history.erase(_equity_history.begin());
		}

		const std::pair<long long, double>& front = _equity_history[0];
		info = AccountInfo(info, front.second, _risk, _asset_count);
		
		// account has lost too much in last interval
		if (info.pl() <= front.second * -_max_loss)
		{
			errorf("%s client '%s' has undergone %f loss in the last %f hours! Closing all position...", _label, _filename, info.pl(), _history_length);
			int failures = 0;
			while (!close_all_positions())
			{
				failures++;
				if (failures >= 5)
				{
					errorf("Client has failed to close all positions (%d) times! Aborting...\n", failures);
					break;
				}
			}
			_live = false;
			errorf("%s client '%s' has gone offline!", _label, _filename);
		}

		return info;
	}

	bool Client::market_order(const std::string& ticker, double amount)
	{
		if (!func_ok("market_order", (void(*)())_market_order)) return false;

		bool res = _market_order(ticker, amount);

		if (!res) flag_error();

		return res;
	}

	double Client::get_shares(const std::string& ticker) const
	{
		if (!func_ok("get_shares", (void(*)())_get_shares)) return 0.0;

		double shares = 0.0;
		bool res = _get_shares(shares, ticker);
		if (!res) flag_error();

		return shares;
	}

	double Client::get_price(const std::string& ticker) const
	{
		if (!func_ok("get_price", (void(*)())_get_price)) return 0.0;

		double price = 0.0;
		bool res = _get_price(price, ticker);
		if (!res) flag_error();

		return price;
	}

	bool Client::close_all_positions()
	{
		// this function can happen when not live

		if (!_close_all_positions)
		{
			errorf("%s: close_all_positions is not bound and cannot be executed!", _filename);
			return false;
		}

		bool res = _close_all_positions();
		if (!res) flag_error();

		return res;
	}

	bool Client::market_open() const
	{
		if (!func_ok("market_open", (void(*)())_market_open)) return false;

		bool open = false;
		bool res = _market_open(open);
		if (!res) flag_error();

		return open;
	}

	bool Client::set_leverage(int multiplier)
	{
		if (!func_ok("set_leverage", (void(*)())_set_leverage)) return false;

		bool res = _set_leverage(multiplier);
		if (!res) flag_error();

		return res;
	}

	std::string Client::to_interval(int interval) const
	{
		if (!func_ok("set_leverage", (void(*)())_set_leverage)) return "";

		const char* interval_str = "";
		bool res = _to_interval(interval_str, interval);
		if (!res) flag_error();

		return std::string(interval_str);
	}
}