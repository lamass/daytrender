#pragma once

#include "../data/candle.h"
#include "../data/accountinfo.h"
#include "../data/assetinfo.h"

#include <string>
#include <vector>

namespace hirzel
{
	class Plugin;
}

namespace daytrender
{
	class Client
	{
	private:
		bool _bound = false;
		mutable bool _live = false;
		bool _shorting_enabled = false;
		int _asset_count = 0;
		double _pl = 0.0;
		double _risk = 0.0;
		double _max_loss = 0.05;
		double _history_length = 24.0;
		int _closeout_buffer = 15 * 60;

		std::vector<std::pair<long long, double>> _equity_history;
		std::string _filename;
		std::string _label;

		hirzel::Plugin* _handle = nullptr;
		
		// init func

		bool (*_init)(const std::vector<std::string>&) = nullptr;

		// api functions

		// non returning
		bool (*_market_order)(const std::string&, double) = nullptr;
		bool (*_close_all_positions)() = nullptr;
		bool (*_set_leverage)(int) = nullptr;

		// returning
		bool (*_get_account_info)(AccountInfo&) = nullptr;
		bool (*_get_candles)(CandleSet&, const std::string&) = nullptr;
		bool (*_get_asset_info)(AssetInfo&, const std::string&) = nullptr;
		bool (*_secs_till_market_close)(int&) = nullptr;
		const char* (*_to_interval)(int) = nullptr;

		// getters

		int (*_key_count)() = nullptr;
		int (*_max_candles)() = nullptr;
		void (*_backtest_intervals)(std::vector<int>&) = nullptr;
		void (*_get_error)(std::string&) = nullptr;
		int (*_api_version)() = nullptr;

		void flag_error() const;
		bool func_ok(const char* label, void(*func)()) const;

	public:
		Client(const std::string& label, const std::string& filepath,
			const std::vector<std::string>& credentials, bool shorting_enabled, double risk,
			double max_loss, int leverage, double history_length, int closeout_buffer);
		~Client();

		void update();

		bool enter_long(const std::string ticker, double pct);
		bool exit_long(const std::string ticker, double pct);
		bool enter_short(const std::string ticker, double pct);
		bool exit_short(const std::string ticker, double pct);

		// api functions

		// non returning
		bool market_order(const std::string& ticker, double amount);
		bool close_all_positions();
		bool set_leverage(int multiplier);

		// returning
		AccountInfo get_account_info() const;
		CandleSet get_candles(const std::string& ticker, int interval, unsigned max, unsigned end) const;
		AssetInfo get_asset_info(const std::string& ticker) const;
		int secs_till_market_close() const;
		std::string to_interval(int interval) const;

		// getters for constants

		int key_count() const { return _key_count(); }
		inline int max_candles() const { return _max_candles(); }

		inline std::vector<int> backtest_intervals() const
		{
			std::vector<int> out;
			_backtest_intervals(out);
			return out;
		}

		inline std::string get_error() const
		{
			std::string error;
			_get_error(error);
			return error;
		}

		// inline getter functions

		inline bool bound() const { return _bound; }
		inline bool is_live() const { return _live; }
		inline bool shorting_enabled() const { return _shorting_enabled; }
		inline const std::string& label() const { return _label; }
		inline const std::string& filename() const { return _filename; }
		inline hirzel::Plugin* handle() const { return _handle; }
		inline double risk() const { return _risk; }
		inline double pl() const { return _pl; }
		inline int asset_count() const { return _asset_count; }
		inline void increment_assets() { _asset_count++; }
	};
}