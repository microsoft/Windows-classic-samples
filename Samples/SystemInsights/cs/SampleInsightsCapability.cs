using System;
using System.Collections.Generic;
using System.Threading;

///
/// Microsoft System Insights namespaces
/// 
using Microsoft.SystemInsights.Common;
using Microsoft.SystemInsights.Capability;

namespace Microsoft.SystemInsights.Samples
{
    /// <summary>
    /// Creates a sample System Insights capability.
    /// This capability demonstrates:
    ///     1. How to register data sources.
    ///     2. How to parse the capability input to make a prediction.
    ///     3. How to cancel the prediction. 
    /// 
    /// <remarks> A DLL should have only one type that implements the ICapability interface.</remarks>
    /// </summary>
    public class SampleInsightsCapability : ICapability, IDisposable
    {
        private CancellationTokenSource cts = new CancellationTokenSource();

        /// 
        /// The assembly version is used if Version is left null. 
        /// 
        public Version Version  => new Version("1.0");

        public string Publisher => "Contoso Corporation";

        /// <summary>
        /// Method to register the data sources required for the capability. 
        /// This method registers a performance counter, an ETW event, and a well-known series. 
        /// </summary>
        /// <returns></returns>
        public CapabilityInformation GetCapabilityInformation()
        {
            DataSource wellKnownSeriesVolume = new DataSource
            {
                SourceType      = DataSourceType.WellKnownSeries,
                WellKnownSeries = new WellKnownSeriesDataSource
                {
                    WellKnownSeriesType = WellKnownSeriesType.Volume
                },
                SeriesContext = "WellKnownSeriesVolume"
            };

            DataSource perfCounter = new DataSource
            {
                SourceType         = DataSourceType.PerfCounter,
                PerformanceCounter = new PerformanceCounterDataSource
                {
                    CounterPath  = "PhysicalDisk",
                    CounterName  = "Disk Bytes/sec",
                    InstanceName = "*"                      // All instances
                },
                AggregationType = DataAggregationType.Sum,
                SeriesContext   = "PhysicalDiskPerfCounters"
            };

            DataSource etwSource = new DataSource
            {
                SourceType = DataSourceType.EventLog,
                Event      = new EventDataSource
                {
                    ChannelName = "System",
                    EventId     = 12                        // System start event after a reboot
                },
                SeriesContext = "SystemStartEvent"
            };

            return new CapabilityInformation(CapabilityPredictionType.Generic,
                                              new List<DataSource> { wellKnownSeriesVolume, perfCounter, etwSource },
                                              "Sample System Insights capability that analyzes data from a well-known series, performance counters, and an ETW event.");
        }

        /// <summary>
        /// Method to write the prediction logic for the capability.
        ///
        /// This sample capability doesn't contain any prediction logic. 
        /// This only shows how to parse the data sources
        /// and return a prediction status.
        /// </summary>
        /// <param name="invokeRequest"> 
        ///     The prediction request object. 
        ///     Dictionary of data containing the data sources specified above. 
        /// </param>
        /// <returns></returns>
        public InvokeResult Invoke(InvokeRequest invokeRequest)
        {
            cts.Token.Register(() => OnCancel());

            ///
            /// The InvokeRequest object is a dictionary containing all of data sources that the capability registered 
            /// during the <see cref="SampleInsightsCapability.GetCapabilityInformation(string)"/> method.
            /// The dictionary uses the SeriesContext that's defined above as they key for each data source.
            /// 
            foreach (var data in invokeRequest.RequestData)
            {
                /// 
                /// Each data source registered above maps directly to a DataSeries object. 
                /// You can use the SeriesContext to identify the data source. 
                ///
                /// An individual data source might record data for multiple instances.
                /// For example, the well-known volume series has data for all volumes.
                /// Or, a performance counter data source can record data for 'all instances'.
                /// The DataSeries object contains structures to help you properly identify 
                /// the data you specified.
                ///
                /// For more details about DataSeries, you can refer to the online documentation, 
                /// which shows you how the DataSeries is constructed for 
                /// each type of data source.
                ///

                string seriesContext                 = data.Key;
                IReadOnlyList<DataSeries> dataSeries = data.Value;

                foreach (DataSeries series in dataSeries)
                {
                    ///
                    /// The Identifier property identifies each instance in the data series.
                    /// 
                    string identifier = series.Identifier;

                    ///
                    /// Each instance may have additional metadata information associated with it, 
                    /// which is contained in the SeriesProperties property. 
                    /// For example, the volume well-known series contains the GUID, drive letter, and friendly name 
                    /// of each volume.
                    /// 
                    IReadOnlyDictionary<string, string> seriesProperties = series.SeriesProperties;

                    ///
                    /// Each DataRecord might contain multiple data points for a given timestamp, 
                    /// such as BytesInbound and BytesOutbound for the networking well-known series.
                    /// The SeriesName object maps the data to the index in the <see cref="DataRecord.Values"/> list.
                    ///
                    /// If you aren't using well-known series, you can ignore this object, as the DataRecord
                    /// only contains one entry per timestamp.
                    /// 
                    IReadOnlyDictionary<string, int> seriesNames = series.SeriesNames;


                    /// 
                    /// The data is contained as a list of <see cref="DataRecord"/>.
                    /// An individual DataRecord contains the data for a given timestamp.
                    /// If the DataRecord contains multiple data points for each timestamp, 
                    /// use the <see cref="SeriesNames"/> object to determine the ordering of the 
                    /// data points.
                    ///
                    IReadOnlyList<DataRecord> dataRecords = series.DataRecords;

                    ///
                    /// You can now use the data from the <see cref="DataRecord.Values"/> to make a prediction.
                    /// 

                }
            }

            ///
            /// Each prediction must return a status, status description, and any associated prediction data. 
            /// 
            /// When returning the associated data, a capability can use different constructors and classes
            /// depending on the prediction type. For example, you can use the ForecastingResult structure to output 
            /// strongly typed data that's associated with a forecasting prediction. 
            /// The advantage of using these types is that the prediction data will be 
            /// displayed nicely in Windows Admin Center, as Windows Admin Center knows how 
            /// to interpret the strongly typed output for each of the prediction types.
            ///
            /// The three different constructors are shown below. Use the one associated with your 
            /// prediction type, which you defined in <see cref="CapabilityInformation.CapabilityPredictionType"/>. 
            /// Because this example is a "Generic" prediction type, this example uses the generic result constructor. 
            ///
            /// Refer to the documentation to see more information about the 
            /// ForecastingResult and AnomalyDetectionResult classes.
            ///

            // return new InvokeResult(PredictionStatus.Ok, "Everything looks good.", new List<ForecastingResult>());
            // return new InvokeResult(PredictionStatus.Ok, "Everything looks good.", new List<AnomalyDetectionResult>());
            return new InvokeResult(PredictionStatus.Ok, "Everything looks good.", "[Your prediction JSON blob goes here]");

        }

        /// <summary>
        /// Method to cancel all inprogress predictions
        /// </summary>
        public void Cancel()
        {
            cts.Cancel();
        }

        private void OnCancel()
        {
            ///
            /// Cancellation logic goes here
            /// 
        }

        public void Dispose()
        {
            cts.Dispose();
        }
    }
}
