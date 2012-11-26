#include "../Configuration/Configuration.h"
#include <fstream>
using namespace std;

bool assertConfig(Configuration& config, char* key, int expectedValue) {
	int actualValue;
	if (!config.get(key, &actualValue)) {
		cerr << "Missing key " << key << endl;
		return false;
	}
	if (actualValue != expectedValue) {
		cerr << "Key " << key << " expected value " << expectedValue << " but got " << actualValue << endl;
		return false;
	}
	return true;
}

int main() {
	ifstream sampleConfiguration("testdata/sample_configuration.txt");
	Configuration config;
	config.load(sampleConfiguration);
	bool success = false;
	success = assertConfig(config, "addsub_delay", 1)
	       &= assertConfig(config, "mul_delay", 3)
	       &= assertConfig(config, "div_delay", 7)
	       &= assertConfig(config, "instruction_q_depth", 4)
	       &= assertConfig(config, "addsub_rs", 2)
	       &= assertConfig(config, "muldiv_rs", 3)
	       &= assertConfig(config, "load_q_depth", 4)
	       &= assertConfig(config, "reorder_buffer", 16)
	       &= assertConfig(config, "ghr_width", 4)
	       &= assertConfig(config, "two_threads_enabled", 1)
	       &= assertConfig(config, "l1_block_size", 64)
	       &= assertConfig(config, "l1_access_delay", 1)
	       &= assertConfig(config, "l1_cache_size", 65536)
	       &= assertConfig(config, "l2_block_size", 128)
	       &= assertConfig(config, "l2_access_delay", 10)
	       &= assertConfig(config, "l2_cache_size", 262144)
	       &= assertConfig(config, "mem_access_delay", 100);
	if (success) {
		cout << "Test succeeded" << endl;
	} else {
		cout << "Test failed" << endl;
	}
}