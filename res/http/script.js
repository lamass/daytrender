var assets = [];
var algorithms = [];

$("document").ready(function () {
	$.get(
		{
			url: "/data",
			success: function (data, status) 
			{
				console.log("Data:", data);
				
				let assetSelect = $("#asset-type-select");
				let tickerSelect = $("#ticker-select");
				let algoSelect = $("#algo-type-select");
				
				for (let i = 0; i < data.assets.length; i++) 
				{
					assetSelect.append(`<option value=${i}>${data.assets[i].label}</option>`);
					for (let j = 0; j < data.assets[i].list.length; j++) 
					{
						assets[i] = [];
						assets[i][j] = data.assets[i].list[j];
					}
				}
				
				for (let i = 0; i < data.algorithms.length; i++)
				{
					algorithms[i] = data.algorithms[i];
					algoSelect.append(`<option value=${i}>${algorithms[i]}</option>`);
				}
				
				let assetType = assetSelect.val();
				
				for (let i = 0; i < assets[assetType].length; i++) {
					let ticker = assets[assetType][i];
					tickerSelect.append(`<option value=${ticker}>${ticker}</option>`);
				}
			}
		});

	$("#asset-type-select").change(() => {
		let assetType = $("#asset-type-select").val();
		let tickerSelect = $("#ticker-select");
		tickerSelect.empty();
		for (let i = 0; i < assets[assetType].length; i++) {
			let ticker = assets[assetType][i];
			tickerSelect.append(`<option value=${ticker}>${ticker}</option>`);
		}
	});

	$("#shutdown-button").click(() => {
		$.get(
			{
				url: "/shutdown",
				success: function (data, status, jqxhr) {
					console.log(data);
					/*
					setTimeout(()=>{
						location.reload();
					}, 12000);
					*/
				}
			});
	});


	var watching = false;
	var watching_interval;

	function get_watch() {
		var assetType = parseInt($("#asset-type-select").val());
		var ticker = $("#ticker-select").val();

		$.get({
			url: "/watch",
			dataType: "json",
			data: {
				"assetType": assetType,
				"ticker": ticker,
			},
			success: function (data, status) {
				console.log("Data:", data);

				var i_traces = [];

				var maxVolume = data.volume[0];
				for (let i = 0; i < data.low.length; i++) {
					maxVolume = (data.volume[i] > maxVolume) ? data.volume[i] : maxVolume;
				}

				var priceTrace = {
					type: "candlestick",
					open: data.open,
					high: data.high,
					low: data.low,
					close: data.close,
					x: data.x,
					yaxis: "y2",
					name: "OHLC",
					increasing: { line: { color: '#00CC00' } },
					decreasing: { line: { color: '#CC0000' } }
				};

				var volumeTrace = {
					x: data.x,
					y: data.volume,
					type: "bar",
					yaxis: "y",
					name: "Volume"
				};


				for (let i = 0; i < data.indicators.length; i++) {
					let ind = data.indicators[i];
					i_traces[i] = {
						x: data.x,
						y: ind.data,
						type: "scatter",
						yaxis: "y2",
						name: ind.name
					};
				}

				var chartdata = [priceTrace, volumeTrace];

				for (let i = 0; i < i_traces.length; i++) {
					chartdata.push(i_traces[i]);
				}

				let title = ticker + " @ " + data.interval + "s";

				var layout = {
					title: title,
					xaxis:
					{
						title: "Time",
						type: "linear"
					},
					yaxis:
					{
						title: "Volume",
						type: "linear",
						range: [0, maxVolume * 2],
						side: "right"
					},
					yaxis2:
					{
						title: "Price",
						overlaying: "y1",
						type: "linear"
					}
				};

				Plotly.newPlot("chart-window", chartdata, layout);

			}
		});
	}

	$("#watch-button").click(() => {
		if (watching) {
			clearInterval(watching_interval);
			watching = false;
			console.log("Stopped watching...");
			return;
		}
		watching = true;
		console.log("Now watching...");
		get_watch();
		watching_interval = setInterval(get_watch, 60000);
	});

	$("#backtest-button").click(() => {
		console.log("backtesting...");
		var assetType = parseInt($("#asset-type-select").val());
		var ticker = $("#ticker-select").val();
		var algoType = parseInt($("#algo-type-select").val());
		var interval = parseInt($("#interval-select").val());
		var window = parseInt($("#window-field").val());
		if (Number.isNaN(window)) {
			window = 0;
		}
		console.log("assetType:", assetType);
		console.log("Ticker:", ticker);
		console.log("algoType:", algoType);
		console.log("Interval:", interval);
		console.log("Window:", window);

		$.get(
			{
				url: "/backtest",
				dataType: "json",
				data: {
					"assetType": assetType,
					"ticker": ticker,
					"algoType": algoType,
					"interval": interval,
					"window": 300
				},
				success: function (data, status) {
					console.log("Response:", data);

					var maxVolume = data.volume[0];
					for (let i = 0; i < data.low.length; i++) {
						maxVolume = (data.volume[i] > maxVolume) ? data.volume[i] : maxVolume;
					}

					var priceTrace = {
						type: "candlestick",
						open: data.open,
						high: data.high,
						low: data.low,
						close: data.close,
						x: data.x,
						yaxis: "y2",
						name: "ohlc",
						increasing: { line: { color: '#00CC00' } },
						decreasing: { line: { color: '#CC0000' } }
					};

					var volumeTrace = {
						x: data.x,
						y: data.volume,
						type: "bar",
						yaxis: "y",
						name: "volume"
					};

					var chartdata = [priceTrace, volumeTrace];

					var layout = {
						xaxis:
						{
							title: "Time",
							type: "linear"
						},
						yaxis:
						{
							title: "Volume",
							type: "linear",
							range: [0, maxVolume * 2],
							side: "right"
						},
						yaxis2:
						{
							title: "Price",
							overlaying: "y1",
							type: "linear"
						}
					};

					Plotly.newPlot("chart-window", chartdata, layout);
				}
			});
	});


});