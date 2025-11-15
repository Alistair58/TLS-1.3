#ifndef BIGMATHS_H
#define BIGMATHS_H

#include <stdint.h>

#define LESS_THAN 1
#define GREATER_THAN 2
#define EQUAL 0

typedef unsigned char uint8;
typedef uint32_t* bignum;

//Re is for reuse - it doesn't return a new pointer; it carries out operations on "a" (the first argument)
//Other functions return a pointer which has been allocated and hence needs to be freed by the caller
//NOTE - Numbers are stored with MSB at index 0 -> Big Endian

//All functions can have an argument as an operand and a destination unless specified otherwise

void printBigNum(char *text, bignum n, int lenN);
bignum createBigNum(bignum a, int len);
void bigNumAdd(bignum a,int lenA,bignum b, int lenB,bignum dest,int lenDest);
void bigNumAddLittle(bignum a,int lenA, uint32_t b,bignum dest,int lenDest);
void bigNumMultiAdd(bignum a,int lenA,bignum b,int lenB,bignum c,int lenC,bignum dest,int lenDest);
void bigNumBitModAdd(bignum a,int lenA,bignum b,int lenB,bignum dest,int lenDest,int bitMod, int carryMult);
void bigNumMult(bignum a,int lenA,bignum b,int lenB,bignum dest,int lenDest);
void bigNumBitModMult(bignum a,int lenA, bignum b,int lenB,bignum dest,int lenDest,int bitMod, int carryMult);
void bigNumModMult(bignum a,int lenA, bignum b,int lenB,bignum n, int lenN,bignum dest,int lenDest);
void bigNumBitMod(bignum a, int lenA,int bitMod,int carryMult,bignum dest,int lenDest);
void bigNumMultByLittle(bignum a,int lenA, uint32_t littleNum,bignum dest,int lenDest);
void bigNumSub(bignum a,int lenA, bignum b,int lenB,bignum dest,int lenDest);
void bigNumBitModMultByLittle(bignum a,int lenA, uint32_t littleNum,bignum dest,int lenDest,int bitMod, int carryMult);
void bigNumSubLittle(bignum a,int lenA, uint32_t b,bignum dest,int lenDest);
void bigNumModSub(bignum a,int lenA,bignum b,int lenB,bignum dest,int lenDest,bignum p,int lenP);
void bigNumBitModInv(bignum a,int lenA,bignum p,int lenP,bignum dest,int lenDest,int bitMod,int carryMult);
void bigNumRShift(bignum a,int lenA,int shift,bignum dest,int lenDest);
void bigNumLShift(bignum a,int lenA,int shift,bignum dest,int lenDest);
void bigNumMod(bignum a,int lenA,bignum n,int lenN,bignum dest,int lenDest);
void bigNumDiv(bignum a,int lenA,bignum b,int lenB,bignum dest,int lenDest);
void bigNumModAdd(bignum a,int lenA, bignum b, int lenB,bignum n,int lenN,bignum dest,int lenDest);

uint8 bigNumCmp(bignum a,int lenA,bignum b,int lenB);
uint8 bigNumCmpLittle(bignum a,int lenA,uint32_t b);

#endif