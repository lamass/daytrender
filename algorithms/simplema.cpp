#define INDICATORS 2

#include <candle.h>
#include <algodefs.h>
#include <iostream>

void EMA(indicator& data, const candleset& candles, int range)
{
	double multiplier = 2.0 / (double)(range + 1);

	double sum = 0.0;
	int initLength = 5;

	for(int i = 0; i < initLength; i++)
	{
		sum += candles[i].close;
	}

	data[0] = sum / (double)initLength;

	for (int i = 1; i < data.size; i++)
	{
		data[i] = candles[i].close * multiplier + data[i - 1] * (1.0 - multiplier);
	}
}

void algorithm (algorithm_data& out)
{
	init_algorithm(out, "Simple MA");
	std::cout << "Start of the algorithm\n";

	const indicator& longma = add_indicator(out, EMA, "long");
	std::cout << "Added longma\n";

	const indicator& shortma = add_indicator(out, EMA, "short");
	std::cout << "Added shortma\n";

	if (shortma.back() > longma.back() && shortma.back(1) < longma.back(1))
	{
		out.buy();
	}
	else if (shortma.back() < longma.back() && shortma.back(1) > longma.back(1))
	{
		out.sell();
	}

	std::cout << "end of algorithm\n";
}
