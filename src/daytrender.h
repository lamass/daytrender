/***********************
 * Author: Isaac Hirzel
 * File: daytrender.h
 * License: MIT
 * 
 ***********************/

#pragma once

#include <string>
#include <vector>

#include "data/paperaccount.h"

#include "data/asset.h"
#include "api/tradeclient.h"


#define PAPER_BY_DEFAULT
#define BACKTESTING
#define JIT_COMPILE_ALGORITHMS

#define ALGORITHM_FOLDER	"/algorithms/"
#define CLIENTS_FOLDER		"/clients/"

namespace daytrender
{
	std::vector<PaperAccount> backtest(int algo_index, int asset_index, const std::vector<int>& ranges);

	void init(const std::string& execpath);
	void free();

	void start();
	void stop();

	bool isRunning();

	std::vector<std::string> getClientInfo();
	std::vector<std::string> getAlgoInfo();
	std::vector<std::pair<std::string, int>> getAssetInfo();
	const Asset* getAsset(int index);
	const TradeClient* getClient(int type);
}