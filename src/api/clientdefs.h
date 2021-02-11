#pragma once

#define CLIENT_API_VERSION 1

#ifndef API_VERSION_CHECK

#ifndef BACKTEST_INTERVALS
#error BACKTEST_INTERVALS must be defined!
#endif

#ifndef MAX_CANDLES
#error MAX_CANDLES must be defined!
#endif

#ifndef KEY_COUNT
#error KEY_COUNT must be defined!
#endif

#include "candle.h"
#include "interval.h"
#include "accountinfo.h"
#include "assetinfo.h"

#include <string>
#include <vector>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>

using namespace daytrender;
using namespace httplib;
using nlohmann::json;

#define JSON_FORMAT "application/json"

std::string error;

bool res_ok(const Result& res)
{
	if (!res)
	{
		error = "failed to get a response";
		return false;
	}
	else if (res->status < 200 || res->status > 299)
	{
		error = "response status was not okay: " + std::to_string(res->status);
		if (!res->body.empty())
		{
			error += ": " + res->body;
		}
		return false;
	}
	else if (res->body.empty())
	{
		error = "response body was empty";
		return false;
	}

	return true;
}

extern "C"
{
	// non-returning functions

	bool init(const std::vector<std::string>& credentials);
	bool market_order(const std::string& ticker, double amount);
	bool close_all_positions();
	bool set_leverage(int numerator);

	// returning functions

	bool get_candles(CandleSet& candles, const std::string& ticker);
	bool get_account_info(AccountInfo& info);
	bool secs_till_market_close(int&);
	bool get_asset_info(AssetInfo&, const std::string& ticker);
	const char* to_interval(int interval);

	// const functions
	int key_count() { return KEY_COUNT; }
	void backtest_intervals(std::vector<int>& out) { out = { BACKTEST_INTERVALS }; }
	int max_candles() { return MAX_CANDLES; }
	int api_version() { return CLIENT_API_VERSION; }

	void get_error(std::string& out)
	{
		out = error;
		error.clear();
	}
}
#endif