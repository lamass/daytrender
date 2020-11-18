#pragma once

#include <vector>

namespace daytrender
{
	struct candle
	{
		unsigned long interval = 0;
		double open = 0.0, high = 0.0, low = 0.0, close = 0.0, volume = 0.0;
		candle() = default;
		candle(double _open, double _high, double _low, double _close, double _volume)
		{
			open = _open;
			high = _high;
			low = _low;
			close = _close;
			volume = _volume;
		}
	};
	typedef std::vector<candle> candleset;

	struct candleset_data
	{
		candleset candles;
		unsigned interval;
	};

}
