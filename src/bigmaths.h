#ifndef BIGMATHS_H
#define BIGMATHS_H


#define LESS_THAN 1
#define GREATER_THAN 2
#define EQUAL 0

typedef unsigned char uint8;
typedef uint32_t* bignum;

//Re is for reuse - it doesn't return a new pointer; it carries out operations on "a" (the first argument)
//Other functions return a pointer which has been allocated and hence needs to be freed by the caller
//NOTE - Numbers are stored with MSB at index 0 -> Big Endian

bignum createBigNum(bignum a, int len);
bignum bigNumAdd(bignum a,int lenA, bignum b, int lenB, int lenDest);
bignum bigNumSubLittle(bignum a,int lenA, uint32_t b,int lenDest);
bignum multiAdd(bignum a,int lenA,bignum b, int lenB, bignum c,int lenC, int lenDest);
bignum bigNumBitModAdd(bignum a,int lenA, bignum b,int lenB,int lenDest,int bitMod, int carryMult);
bignum bigNumMult(bignum a,int lenA, bignum b,int lenB,int lenDest);
void bigNumMultRe(bignum a,int lenA, bignum b,int lenB);
bignum bigNumBitModMult(bignum a,int lenA, bignum b,int lenB,int lenDest,int bitMod, int carryMult);
bignum bigNumModMult(bignum a,int lenA, bignum b,int lenB,bignum n, int lenN);
bignum bigNumModMultRe(bignum a,int lenA, bignum b,int lenB,bignum n, int lenN);
bignum bigNumSub(bignum a,int lenA, bignum b,int lenB,int lenDest);
void bigNumSubRe(bignum a,int lenA,bignum b,int lenB);
bignum bigNumSubLittle(bignum a,int lenA, uint32_t b,int lenDest);
bignum bigNumModSub(bignum a,int lenA, bignum b,int lenB,int lenDest,bignum p,int lenP);
bignum bigNumBitMod(bignum a, int lenA,int bitMod,int carryMult, int lenDest);
bignum bigNumBitModInv(bignum a, int lenA,bignum p, int lenP, int lenDest,int bitMod,int carryMult);
bignum bigNumMultByLittle(bignum a,int lenA, uint32_t littleNum,int lenDest);
bignum bigNumBitModMultByLittle(bignum a,int lenA,uint32_t littleNum,int lenDest,int bitMod, int carryMult);
bignum bigNumRShift(bignum a,int lenA,int shift);
void bigNumRShiftRe(bignum a,int lenA,int shift); 
bignum bigNumLShift(bignum a,int lenA,int shift);
void bigNumLShiftRe(bignum a,int lenA,int shift); 
uint8 bigNumCmp(bignum a,int lenA,bignum b,int lenB);
bignum bigNumMod(bignum a,int lenA,bignum n,int lenN);
void bigNumModRe(bignum a,int lenA,bignum n,int lenN);
bignum bigNumDiv(bignum a,int lenA,bignum b,int lenB);
void bigNumDivRe(bignum a,int lenA,bignum b,int lenB);
bignum bigNumModAdd(bignum a,int lenA, bignum b, int lenB,bignum n,int lenN);

#endif