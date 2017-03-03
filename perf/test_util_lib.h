#include <boost/random/mersenne_twister.hpp>

namespace br = boost::random;

int GenRandomBytes(
	unsigned char*	pb,
	size_t			cb,
	br::mt19937		&rng);

int HashRandomBytes(
	size_t			cntBytesToHash,
	unsigned int	uiSeed);

int UBCVerifyRandomBytes(
	size_t			cntBytesToHash,
	unsigned int	uiSeed);
