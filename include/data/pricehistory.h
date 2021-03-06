#ifndef DAYTRENDER_PRICEHISTORY_H
#define DAYTRENDER_PRICEHISTORY_H

// local includes
#include <data/candle.h>


namespace daytrender
{
	class PriceHistory
	{
	private:
		Candle* _data = nullptr;
		unsigned _size = 0;
		unsigned _interval = 0;
		bool _slice = false;

		// constructor for making slices
		PriceHistory(Candle* parent_data, unsigned parent_size,
			unsigned parent_interval, unsigned offset, unsigned size);

	public:
		PriceHistory() = default;
		PriceHistory(unsigned size, unsigned interval);
		PriceHistory(PriceHistory&& other);
		PriceHistory(const PriceHistory& other);
		~PriceHistory();

		PriceHistory& operator=(const PriceHistory& other);

		PriceHistory slice(unsigned offset, unsigned size) const
		{
			if (!_data || offset + size > _size) return {};
			return PriceHistory(_data, _size, _interval, offset, size);
		}


		const Candle& get(unsigned index) const 
		{
			if (index >= _size) return *_data;
			return _data[index];
		}


		Candle& get(unsigned index)
		{
			// this account for shamt but allows for only one check
			if (index >= _size) return *_data;
			return _data[index];
		}


		inline Candle& operator[](int index)
		{
			return get(index);
		}


		inline const Candle& operator[] (unsigned index) const
		{
			return get(index);
		}


		inline const Candle& back(unsigned index = 0) const
		{
			return get((_size - 1) - index);
		}


		inline const Candle& front(unsigned index = 0) const
		{
			return get(index);
		}


		inline bool is_slice() const { return _slice; }
		inline bool empty() const { return _size == 0; }
		inline unsigned size() const { return _size; }
		inline int interval() const { return _interval; }
	};
}

#endif
