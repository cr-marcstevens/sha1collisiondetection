#include <string.h>

#include <iostream>
#include <vector>

#include "sha1.h"

#include "test_util_lib.h"

#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>

namespace po = boost::program_options;

using namespace std;

#define UNUSED_PARAMETER(__X__) (void)(__X__)

typedef struct _HashTimingRecord
{
	size_t	cntBytes;
	size_t	cntIterations;
	boost::timer::cpu_times	timesHashing;
	boost::timer::cpu_times	timesOverhead;
} HashTimingRecord;

void TimeHashing(
	unsigned char	*pb,
	size_t			cb,
	size_t			cntIterations,
	boost::timer::cpu_times	*pTimeElapsed,
	bool			fUBCCheck)
{
	SHA1_CTX ctx;
	unsigned char hash[20];

	boost::timer::cpu_timer t;

	unsigned char	*pbLocal = pb;

	t.start();

	for (size_t i = 0; i < cntIterations; i++)
	{
		SHA1DCInit(&ctx);
		if (!fUBCCheck)
		{
			SHA1DCSetUseUBC(&ctx, 0);
		}
		SHA1DCUpdate(&ctx, (char*)pbLocal, cb);
		SHA1DCFinal(hash, &ctx);
		pbLocal += cb;
	}

	t.stop();

	*pTimeElapsed = t.elapsed();

}

int EmptySHA1Update(
	SHA1_CTX*	ctx,
	unsigned char*	pb,
	size_t			cb)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(pb);
	UNUSED_PARAMETER(cb);

	return 0;
}

int EmptySHA1Final(
	unsigned char	hash[20],
	SHA1_CTX*		ctx)
{
	UNUSED_PARAMETER(hash);
	UNUSED_PARAMETER(ctx);

	return 0;
}

void TimeLoopOverhead(
	size_t	cntIterations,
	boost::timer::cpu_times	*pTimeElapsed,
	bool fUBCCheck)
{
	SHA1_CTX	ctx;
	unsigned char*	pb = NULL;
	size_t			cb = 0;
	unsigned char	hash[20] = { 0 };

	boost::timer::cpu_timer t;

	t.start();

	for (size_t i = 0; i < cntIterations; i++)
	{
		SHA1DCInit(&ctx);
		if (!fUBCCheck)
		{
			SHA1DCSetUseUBC(&ctx, 0);
		}
		EmptySHA1Update(&ctx, pb, cb);
		EmptySHA1Final(hash, &ctx);
	}

	t.stop();

	*pTimeElapsed = t.elapsed();
}

void TimeNoDetectHashing(
    unsigned char	*pb,
    size_t			cb,
    size_t			cntIterations,
    boost::timer::cpu_times	*pTimeElapsed,
    bool			fUBCCheck)
{
    SHA1_CTX ctx;
    unsigned char hash[20];

    boost::timer::cpu_timer t;

    unsigned char	*pbLocal = pb;

    t.start();

    for (size_t i = 0; i < cntIterations; i++)
    {
        SHA1DCInit(&ctx);
        SHA1DCSetUseDetectColl(&ctx, 0);
        SHA1DCUpdate(&ctx, (char*)pbLocal, cb);
        SHA1DCFinal(hash, &ctx);
        pbLocal += cb;
    }

    t.stop();

    *pTimeElapsed = t.elapsed();

}

int GenRandomAndPerformHashTimings(
	size_t	cntBytes,
	size_t	cntIterations,
	br::mt19937 rng,
	HashTimingRecord *pHashTimingRecord,
	bool fUBCCheck)
{
	unsigned char*	pb = NULL;

	int ret = -1;

	pb = (unsigned char*)malloc(cntBytes*cntIterations);
	if (NULL == pb)
	{
		ret = -1;
		goto Cleanup;
	}

	pHashTimingRecord->cntBytes = cntBytes;
	pHashTimingRecord->cntIterations = cntIterations;

	GenRandomBytes(pb, cntBytes*cntIterations, rng);

	cout << "Hashing " << cntBytes << " bytes of data in " << cntIterations << " iterations " << endl;
	TimeHashing(pb, cntBytes, cntIterations, &pHashTimingRecord->timesHashing, fUBCCheck);
	cout << "Loop Overhead of " << cntIterations << "iterations" << endl;
	TimeLoopOverhead(cntIterations, &pHashTimingRecord->timesOverhead, fUBCCheck);

	ret = 0;

Cleanup:

	if (pb)
	{
		free(pb);
	}

	return ret;

}

int GenRandomAndPerformHashTimings_NoDetect(
    size_t	cntBytes,
    size_t	cntIterations,
    br::mt19937 rng,
    HashTimingRecord *pHashTimingRecord,
    bool fUBCCheck)
{
    unsigned char*	pb = NULL;

    int ret = -1;

    pb = (unsigned char*)malloc(cntBytes*cntIterations);
    if (NULL == pb)
    {
        ret = -1;
        goto Cleanup;
    }

    pHashTimingRecord->cntBytes = cntBytes;
    pHashTimingRecord->cntIterations = cntIterations;

    GenRandomBytes(pb, cntBytes*cntIterations, rng);

    cout << "Hashing " << cntBytes << " bytes of data in " << cntIterations << " iterations " << endl;
    TimeNoDetectHashing(pb, cntBytes, cntIterations, &pHashTimingRecord->timesHashing, fUBCCheck);
    cout << "Loop Overhead of " << cntIterations << "iterations" << endl;
    TimeLoopOverhead(cntIterations, &pHashTimingRecord->timesOverhead, fUBCCheck);

    ret = 0;

Cleanup:

    if (pb)
    {
        free(pb);
    }

    return ret;

}

int main(int argc, char** argv)
{
	unsigned int uiSeed;

	string strCounts;
	string strSeed;

	boost::char_separator<char> sep(",;");

	int ret = -1;

	bool fUBCCheck = false;

	po::options_description desc("Allowed options");

	desc.add_options()
		("help,h", "Show options")
		("counts,c", po::value<string>(&strCounts)->default_value("32,64,128,256"), "Count of bytes to hash.")
		("seed,s", po::value<string>(&strSeed), "Seed to use for rand.")
		("UBC,u", "Enable Unavoidable Bit Condition checks.");
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
	po::notify(vm);

	boost::tokenizer<boost::char_separator<char> > tokens(strCounts, sep);
	boost::tokenizer<boost::char_separator<char> >::iterator cur_token;

	br::mt19937		rng;

	vector<HashTimingRecord>	records;
	vector<HashTimingRecord>	records_no_detect;

	HashTimingRecord	*phash_timing_record = NULL;
	HashTimingRecord	*phash_timing_record_no_detect = NULL;

	if (0 < vm.count("help"))
	{
		cout << desc << endl;
		ret = 0;
		goto Cleanup;
	}

	if (0 < vm.count("seed"))
	{
		
		uiSeed = atoi(strSeed.c_str());
		cout << "Using specified seed: " << uiSeed << "." << endl;
	}
	else
	{
		uiSeed = (unsigned int)time(NULL);
		cout << "No random seed specified, using time(Null):" << uiSeed << "." << endl;
	}

	if (0 < vm.count("UBC"))
	{
		fUBCCheck = true;
		cout << "Performing unavoidable bit condition checks." << endl;
	}

	rng = br::mt19937(uiSeed);

	phash_timing_record = (HashTimingRecord*)malloc(sizeof(HashTimingRecord));
	phash_timing_record_no_detect = (HashTimingRecord*)malloc(sizeof(HashTimingRecord));

	for (cur_token = tokens.begin(); cur_token != tokens.end(); ++cur_token)
	{
		size_t cntBytes = (size_t)atoi(cur_token.current_token().c_str());
		size_t cntIterations = (size_t)ceil((double)(1 << 24) / cntBytes);
		cout << "Performing timing for: <" << cntBytes << "> ";

		GenRandomAndPerformHashTimings(cntBytes, cntIterations, rng, phash_timing_record, fUBCCheck);
		GenRandomAndPerformHashTimings_NoDetect(cntBytes, cntIterations, rng, phash_timing_record_no_detect, fUBCCheck);

		records.push_back(*phash_timing_record);
 		records_no_detect.push_back(*phash_timing_record_no_detect);
    	}
	cout << endl;

	// top of columns
	printf("bytes\titerations\thashing wall\thashing user\thashing system\thashing total\toverhead wall\toverhead user\toverhead system\toverhead total\n");

	for (vector<HashTimingRecord>::iterator rec = records.begin(); rec != records.end(); rec++)
	{
		printf("%lu\t%lu\t\t", rec->cntBytes, rec->cntIterations);
		printf("%li\t%li\t%li\t\t%li\t", rec->timesHashing.wall, rec->timesHashing.user, rec->timesHashing.system, (rec->timesHashing.user + rec->timesHashing.system));
		printf("%li\t\t%li\t\t%li\t\t%li\n", rec->timesOverhead.wall, rec->timesOverhead.user, rec->timesOverhead.system, (rec->timesOverhead.user + rec->timesOverhead.system));
	}

	// top of columns
	printf("bytes\titerations\thashing wall\thashing user\thashing system\thashing total\toverhead wall\toverhead user\toverhead system\toverhead total\n");

	for (vector<HashTimingRecord>::iterator rec = records_no_detect.begin(); rec != records_no_detect.end(); rec++)
	{
		printf("%lu\t%lu\t\t", rec->cntBytes, rec->cntIterations);
		printf("%li\t%li\t%li\t\t%li\t", rec->timesHashing.wall, rec->timesHashing.user, rec->timesHashing.system, (rec->timesHashing.user + rec->timesHashing.system));
		printf("%li\t\t%li\t\t%li\t\t%li\n", rec->timesOverhead.wall, rec->timesOverhead.user, rec->timesOverhead.system, (rec->timesOverhead.user + rec->timesOverhead.system));
	}

	ret = 0;

Cleanup:

	if (phash_timing_record)
	{
		free(phash_timing_record);
	}

	return ret;
}
