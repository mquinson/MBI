/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file GtiChannelId.hpp
 *       @see gti::GtiChannelId
 *
 * @author Tobias Hilbrich
 * @date 7.12.2010
 */

#ifndef UINT64_C
#define UINT64_C(c)  c ## ULL
#endif

//=============================
// GtiChannelId
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
GtiChannelId<N64, NLevels,NBitsPerLevel>::GtiChannelId (int numUsed)
 : myNumUsed (numUsed)
{
	/*Nothing to do*/
	/*
	 * Is initialized by a series of set64 calls
	 */
}

//=============================
// ~GtiChannelId
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
GtiChannelId<N64, NLevels,NBitsPerLevel>::~GtiChannelId ()
{
	/*Nothing to do*/
}

//=============================
// set64
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
void GtiChannelId<N64, NLevels,NBitsPerLevel>::set64 (int n, uint64_t bits)
{
	assert (n < N64 && n >= 0);
	data[n] = bits;
}

//=============================
// get64
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
uint64_t GtiChannelId<N64, NLevels,NBitsPerLevel>::get64 (int n)
{
	assert (n < N64 && n >= 0);
	return data[n];
}

//=============================
// setSubId
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
void GtiChannelId<N64, NLevels,NBitsPerLevel>::setSubId (int level, uint64_t channel)
{
	setValueIndexed (level*2, channel);
}

//=============================
// getSubId
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
long GtiChannelId<N64, NLevels,NBitsPerLevel>::getSubId (int level)
{
	return getValueIndexed (level*2);
}

//=============================
// isEqual
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
bool GtiChannelId<N64, NLevels,NBitsPerLevel>::isEqual (I_ChannelId* other)
{
	GtiChannelId<N64, NLevels,NBitsPerLevel> *otherCast = (GtiChannelId<N64, NLevels,NBitsPerLevel>*) other;

	for (int i = 0; i < N64; i++)
	{
		if (data[i] != otherCast->get64(i))
			return false;
	}

	return true;
}

//=============================
// isLessThan
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
bool GtiChannelId<N64, NLevels,NBitsPerLevel>::isLessThan (I_ChannelId* other)
{
	for (int i = NLevels-1; i >= 0; i--)
	{
		uint64_t a = getSubId(i),
								b = other->getSubId(i);

		if (a == b)
			continue;

		return a < b;
	}

	//If all equal its not less than compare the offset/stride bits
	uint32_t o1, o2, s1, s2;
	isStrideRepresentation (&o1, &s1);
	other->isStrideRepresentation (&o2, &s2);
	if (o1 != o2)
	    return o1 < o2;

	return s1 < s2;
}

//=============================
// copy
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
I_ChannelId* GtiChannelId<N64, NLevels,NBitsPerLevel>::copy (void)
{
	GtiChannelId<N64, NLevels,NBitsPerLevel> *ret = new GtiChannelId<N64, NLevels,NBitsPerLevel> (myNumUsed);

	for (int i = 0; i < N64; i++)
	{
		ret->set64 (i, data[i]);
	}

	return ret;
}

//=============================
// toString
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
std::string GtiChannelId<N64, NLevels,NBitsPerLevel>::toString (void)
{
	std::string ret = "";

	for (int i = 0; i < myNumUsed; i++)
	{
		if (i != 0)
			ret = "." + ret;

		long sid = getSubId (i);

		char temp[128];
		sprintf (temp, "%ld", sid);

		ret = temp + ret;
	}

	return ret;
}

//=============================
// getNumLevels
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
int GtiChannelId<N64, NLevels,NBitsPerLevel>::getNumLevels (void)
{
	return NLevels;
}

//=============================
// getNumUsedSubIds
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
int GtiChannelId<N64, NLevels,NBitsPerLevel>::getNumUsedSubIds (void)
{
	return myNumUsed;
}

//=============================
// setSubIdNumChannels
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
void GtiChannelId<N64, NLevels,NBitsPerLevel>::setSubIdNumChannels (int level, uint64_t numChannels)
{
	setValueIndexed (level*2 + 1, numChannels);
}

//=============================
// getSubIdNumChannels
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
long GtiChannelId<N64, NLevels,NBitsPerLevel>::getSubIdNumChannels (int level)
{
	return getValueIndexed (level*2 + 1);
}

//=============================
// setValueIndexed
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
void GtiChannelId<N64, NLevels,NBitsPerLevel>::setValueIndexed (int index, uint64_t value)
{
	assert ( index >= 0 && index < 2*NLevels );

	//Increment "value"
	//We us 0 to represent "not set"
	value++;

	//calculate indices
	int startIndex = index * NBitsPerLevel;
	int endIndex = startIndex + NBitsPerLevel;

	int index64Begin = startIndex/64;
	int index64End = endIndex/64;
	startIndex = startIndex%64;
	endIndex = endIndex%64;

	//Build bitmask to null out old content
	uint64_t bitmask = (UINT64_C(0xFFFFFFFFFFFFFFFF) >> (64-startIndex));

	if (index64Begin == index64End)
		bitmask = bitmask | (UINT64_C(0xFFFFFFFFFFFFFFFF) << endIndex);

	//Put new content to the right place
	uint64_t temp = ((uint64_t) value) << startIndex;

	//Null out with bitmask and add new content
	data[index64Begin] = (data[index64Begin] & bitmask) +  temp;

	//Do we overlapp into a second 64bit ?
	if (index64End != index64Begin && endIndex > 0)
	{
		//We only support channel ids smaller than 64 bits ....
		assert (index64End == index64Begin + 1);

		temp = ((uint64_t)value) >> (64 - startIndex);
		bitmask = UINT64_C(0xFFFFFFFFFFFFFFFF) << endIndex;

		data[index64End] = (data[index64End] & bitmask) +  temp;
	}

	//DEBUG:
	//printf ("setSubId %d %lu: ",  index, value);
	//for (int x = 0; x < N64; x++)
	//	printf ("0x%lX ", data[x]);
	//printf ("\n");
}

//=============================
// getValueIndexed
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
long GtiChannelId<N64, NLevels,NBitsPerLevel>::getValueIndexed (int index)
{
	assert (index >= 0 && index < 2*NLevels);

	//calculate indices
	int startIndex = index * NBitsPerLevel;
	int endIndex = startIndex + NBitsPerLevel;

	int index64Begin = startIndex/64;
	int index64End = endIndex/64;
	startIndex = startIndex%64;
	endIndex = endIndex%64;

	//Build bitmask to null out rest of content
	uint64_t bitmask = 0;
	if (startIndex != 0)
		bitmask = (UINT64_C(0xFFFFFFFFFFFFFFFF) >> (64-startIndex));

	if (index64Begin == index64End)
		bitmask = bitmask | (UINT64_C(0xFFFFFFFFFFFFFFFF) << endIndex);
	bitmask = ~bitmask; //invert

	//Get the content from internal data, null out not needed data, shift to the right place
	uint64_t temp = (data[index64Begin] & bitmask) >> startIndex;

	//Do we overlap into a second 64bit ?
	if (index64End != index64Begin && endIndex > 0)
	{
		//We only support channel ids smaller than 64 bits ....
		assert (index64End == index64Begin + 1);

		bitmask = UINT64_C(0xFFFFFFFFFFFFFFFF) << endIndex;
		bitmask = ~bitmask;

		temp += (data[index64End] & bitmask) << (64 - startIndex);
	}

	//decrement temp if > 0, if 0 return -1
	if (temp == 0)
		return -1;

	temp--;
	return temp;
}

//=============================
// setStrideRepresentation
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
void GtiChannelId<N64, NLevels,NBitsPerLevel>::setStrideRepresentation (uint32_t offset, uint32_t stride)
{
    data[N64-1] = (((uint64_t)offset) << 32) + (uint64_t)stride;
}

//=============================
// isStrideRepresentation
//=============================
template <int N64, int NLevels, int NBitsPerLevel>
bool GtiChannelId<N64, NLevels,NBitsPerLevel>::isStrideRepresentation (uint32_t *outOffset, uint32_t *outStride)
{
    uint32_t offset = (uint32_t)(data[N64-1] >> 32);
    uint32_t stride = (uint32_t)(data[N64-1] & 0x00000000FFFFFFFF);

    if (stride == 0)
    {
        if (outOffset) *outOffset = 0;
        if (outStride) *outStride = 1;
        return false;
    }

    if (outOffset) *outOffset = offset;
    if (outStride) *outStride = stride;

    return true;
}

/*EOF*/
