#include "error_handling.h"

#include "rover.h"
#include "scanline.h"

#include <chrono>
#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>

constexpr char c_szHELP[] = "help";
constexpr char c_szPORT[] = "port";
constexpr char c_szLIDAR[] = "lidar";
constexpr char c_szLOG[] = "log";
constexpr char c_szMANUAL[] = "manual";
constexpr char c_szMAP[] = "map";

constexpr char c_szINPUT[] = "input-file";
constexpr char c_szVIDEO[] = "video";
constexpr char c_szOUTPUT[] = "out";

int ParseLogFile(std::ifstream& ifs, bool bVideo, boost::optional<std::string> const& ostrOutput);
int ConnectToRobot(std::string const& strPort, std::string const& strLidar, std::ofstream& ofsLog, bool bManual, boost::optional<std::string> const& ostrOutput);

int main(int nArgs, char* aczArgs[]) {
	namespace po = boost::program_options;

	// We currently support two input modes
	// - when 'input-file' is specified, we read the saved log data and can create
	//	 a map from that log data without powering up the robot. Good for testing
	//	 algorithms.
	// - otherwise connect to microcontroller via serial port 'port'
	//	 Currently, the robot can be controlled manually using the WASD keys and 
	//	 the robot controller will send the sensor data which can be saved for
	//	 later analysis
	po::options_description optdescGeneric("Allowed options");
	optdescGeneric.add_options()
	    (c_szHELP, "Print help message")
	    (c_szPORT, po::value<std::string>()->value_name("p"), "Connect to robot on port <p>")
	    (c_szLIDAR, po::value<std::string>()->value_name("l"), "Connect to Lidar sensor on port <p>")
	    (c_szINPUT, po::value<std::string>()->value_name("file"), "Read sensor data from input file <file>");

	po::options_description optdescRobot("Robot options");
	optdescRobot.add_options()
	    (c_szLOG, po::value<std::string>()->value_name("file"), "Log all sensor data to <file>")
	    (c_szMANUAL, "Control robot manually via AWSD keys")
        (c_szMAP, po::value<std::string>()->value_name("file"), "Write map to <file>");
    
    po::options_description optdescInputFile("Input File Options");
	optdescInputFile.add_options()
	    (c_szVIDEO, "If specified, a video of path will be written instead of map image")
        (c_szOUTPUT, po::value<std::string>()->value_name("file"), "Write output to <file>");
    
    po::options_description optdesc;
    optdesc.add(optdescGeneric).add(optdescRobot).add(optdescInputFile);
    
	po::variables_map vm;
	po::store(po::parse_command_line(nArgs, aczArgs, optdesc), vm);
	po::notify(vm);    
	
	if(vm.count(c_szHELP)) {
		std::cout << optdesc << std::endl;
		return 0;
	} else if(vm.count(c_szINPUT)) {
		// Read saved sensor data from log file 
		auto const strLogFile = vm[c_szINPUT].as<std::string>();
		std::ifstream ifs(strLogFile.c_str());
		if(!ifs) {
			std::cerr << "Couldn't open " << vm[c_szINPUT].as<std::string>() << std::endl;
			return 1;
		}

        bool const bVideo = vm.count(c_szVIDEO);
        boost::optional<std::string> ostrOutput = vm.count(c_szOUTPUT)
             ? boost::make_optional(vm[c_szOUTPUT].as<std::string>())
             : boost::none;
        
         return ParseLogFile(ifs, bVideo, ostrOutput);		
	} else if(vm.count(c_szPORT) && vm.count(c_szLIDAR)) {
		// Read serial port, log file name etc
		auto const strPort = vm[c_szPORT].as<std::string>();
		auto const strLidar = vm[c_szLIDAR].as<std::string>();
		
		std::basic_ofstream<char> ofsLog;
		if(vm.count(c_szLOG)) {
			ofsLog.open(vm[c_szLOG].as<std::string>(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
			VERIFY(ofsLog.good());
		}
		bool const bManual = vm.count(c_szMANUAL);
        
        boost::optional<std::string> strOutput;
        if(vm.count(c_szMAP)) {
			strOutput = vm[c_szMAP].as<std::string>();
		}
        return ConnectToRobot(strPort, strLidar, ofsLog, bManual, strOutput);
	} else {
		std::cerr << "You must specify either the port to read from or an input file to parse" << std::endl;
		std::cerr << optdesc << std::endl;
		return 1;
	}
}
