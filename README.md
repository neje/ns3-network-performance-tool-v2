# ns3-network-performance-tool-v2
NS-3 network performance analysis tool on application layer. Calculates E2E packet delay (histogram, min, max, average, median, jitter), packet loss, throughput.

INSTALLATION INSTRUCTIONS:
Copy files from repository to ns-3 installation folder. Nothing else is needed to be done.
These folders are changed:
.../scratch,
.../src/applications,
.../src/applications/model,
.../src/applications/helper

There are few small differences between ns3 releases, so files that are release specific are stored in  the folder: "release specific files". 

USAGE INSTRUCTIONS:
Usage of StatsFlows tool in NS-3 script is very easy. Users should define object of the class StatsFlows after configuring all nodes in the network and before starting the simulation process (by calling the Simulator::Run() function). At the end of simulation all data that are collected in the StatsFlows statistics object should be written into output file. This is done by using StatsFlows member function Finalize() before destroying all simulation objects: 
int main () {
  // .... network configuration
  StatsFlows sf;
  Simulator::Run (); 
  sf.Finalize ();
  Simulator::Destroy ();
  return 0;
}
For more detailed instructions see example test.cc in the scratch folder.
For multiple iteration runs see example multi-run.cc in the scratch folder. We do not recommend the automatic control of the RNG RUN number for multiple simulation iterations due to potential memory problems. Instead, it is more convenient to control the RNG RUN number externally using the appropriate Linux bash script. See example multi-run.sh. Runing the bash script is done by using the command (first copy the file in the ns-3 root folder and edit the properties of the multi-run.sh file to be executable):
./multi-run.sh

OUTPUT FILES:
There are three types of output files: vector, scalar and summary file. In this software release, all data are recorded in the text files using a comma as a separator, thus obtaining comma separated value (CSV) files that are easily loaded and processed in the MS Excel, Matlab, or any other similar program for creating charts or for further data analysis. 
One vector file is produced for every simulation run and it contains time series data for every received packet: reception time, flow ID, sequence number and delay. Since the amount of data generated in this way is potentially very large (proportional to the total number of received packets in the network), the entry of data into the vector file is disabled by default.
One scalar file is also produced at the end of each simulation run and this file contains the scalar data for every detected packet flow: throughput, packet loss ratio, E2E delay (min, max, median and average) and jitter. Scalar results are available for every individual flow, but are also averaged for all flows and given at the end of the file. It should be noted that all packets are also statistically processed regardless of network flows, so the users can chose between statistics based on averaging of all network flows (AAF) or based on averaging of all packets (AAP). Writing to scalar files is enabled by default as scalar data occupy relatively little memory space.
Within this software tool we have also provided a generic simulation script in order to allow easier control of multiple simulation runs. Every simulation run writes one line of averaged scalar data for that run to the common output summary file. Therefore, at the end of simulation series, users have all results in one summary file. Interested users can look at the implementation of the multi-run.cc script for more details about summary file.
Users can control file names and chose whether or not to enable writing to scalar or vector files using the constructor's arguments of the StatsFlows object:
StatsFlows sf (uint64_t runNo, std::string fileNamePrefix, bool scalarFileWriteEnable, bool vectorFileWriteEnable);
The file names are given in the form: /fileNamePrefix/-run/runNo/-vec.csv for the vector files, /fileNamePrefix/-run/runNo/-sca.csv for the scalar files, /fileNamePrefix/-summary.csv for the summary file. 


