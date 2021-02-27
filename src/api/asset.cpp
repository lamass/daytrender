#include "asset.h"

#include "paperaccount.h"
#include "../data/mathutil.h"
#include "../interface/interface.h"

#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

namespace daytrender
{
	Asset::Asset(const Strategy* strategy, const std::string& ticker, int interval,
		const std::vector<int>& ranges) :
	_strategy(strategy),
	_interval(interval),
	_ticker(ticker),
	_ranges(ranges),
	_live(true)
	{
		for (int i = 0; i < _ranges.size(); i++)
		{
			if (_ranges[i] > _candle_count) _candle_count = _ranges[i];
		}
		_candle_count += _strategy->data_length();

		PaperAccount acc = interface::backtest(this);
		double kelly = acc.kelly_criterion();
		_risk = (kelly >= 0.0 ? kelly : 0.0);
	}
	
	void Asset::update(Client& client)
	{
		// if the client or asset is currently not live, do not update
		if (!client.is_live() || !_live) return;

		// if the proper amount of time has not passed, do not update
		if ((hirzel::sys::get_seconds() - _last_update) < _interval) return;
		// updating previously updated time
		long long curr_time = hirzel::sys::get_seconds();
		_last_update = curr_time - (curr_time % _interval);

		INFO("Updating %s...", _ticker);

		// getting candlestick data from client
		CandleSet candles = get_candles(client);
		// 
		if (candles.error())
		{
			ERROR("%s: CandleSet error: %s", _ticker, candles.error());
			return;
		}

		// processing the candlestick data gotten from client
		_data = _strategy->execute(candles, _ranges);

		// error handling
		if (_data.error())
		{
			ERROR("%s: Algorithm: %s", _ticker, _data.error());
			return;
		}

		if (_data.candles().error())
		{
			ERROR("%s: Algorithm candles: %s", _ticker, _data.candles().error());
			return;
		}
		
		bool res = false;
		switch (_data.action())
		{
		case ENTER_LONG:
			res = client.enter_long(_ticker, _risk);
			break;

		case EXIT_LONG:
			res = client.exit_long(_ticker);
			SUCCESS("%s: Exited long position", _ticker);
			break;

		case ENTER_SHORT:
			res = client.enter_short(_ticker, _risk);
			SUCCESS("%s: Entered short position", _ticker);
			break;

		case EXIT_SHORT:
			res = client.exit_short(_ticker);
			SUCCESS("%s: Exited short position", _ticker);
			break;

		case NOTHING:
			res = true;
			SUCCESS("%s: No action taken", _ticker);
			break;

		default:
			ERROR("%s: Invalid action received from strategy: %d", _ticker, _data.action());
			_live = false;
			break;
		}

		if (!res)
		{
			_live = false;
			ERROR("%s: Failed to handle action. Asset is no longer live.", _ticker);
		}
	}
}