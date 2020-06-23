/*
*	Список загружаемых слов 0 уровня.
*/

#ifndef _WORDL0_H_
#define _WORDL0_H_

int wl0_SH(nicenv_t * e);

int wl0_SP(nicenv_t * e);
int wl0_CS(nicenv_t * e);
int wl0_EQ(nicenv_t * e);
int wl0_QE(nicenv_t * e);
int wl0_VALU(nicenv_t * e);
int wl0_DUP(nicenv_t * e);
int wl0_DUPS(nicenv_t * e);
int wl0_DUPV(nicenv_t * e);
int wl0_FLIP(nicenv_t * e);
int wl0_FLOP(nicenv_t * e);
int wl0_PUSH(nicenv_t * e);
int wl0_POP(nicenv_t * e);
int wl0_ADD(nicenv_t * e);
int wl0_SUB(nicenv_t * e);
int wl0_NEGA(nicenv_t * e);
int wl0_DIV(nicenv_t * e);
int wl0_MUL(nicenv_t * e);
int wl0_MOD(nicenv_t * e);
int wl0_DIM(nicenv_t * e);
int wl0_IDX(nicenv_t * e);

int wl0_VAR(nicenv_t * e);
int wl0_VARL(nicenv_t * e);
int wl0_TYPL(nicenv_t * e);
int wl0_LIST(nicenv_t * e);
int wl0_LSS(nicenv_t * e);
int wl0_SHOW(nicenv_t * e);
int wl0_RUN(nicenv_t * e);
int wl0_LOOP(nicenv_t * e);
int wl0_REPE(nicenv_t * e);
int wl0_TRAN(nicenv_t * e);
int wl0_COMP(nicenv_t * e);
int wl0_RAND(nicenv_t * e);
int wl0_LAST(nicenv_t * e);
int wl0_PREV(nicenv_t * e);
int wl0_XIST(nicenv_t * e);
int wl0_IF(nicenv_t * e);
int wl0_ELSE(nicenv_t * e);

int wl0_DROP(nicenv_t * e);
int wl0_DR(nicenv_t * e);
int wl0_SWAP(nicenv_t * e);
int wl0_ROT(nicenv_t * e);
int wl0_TYPE(nicenv_t * e);
int wl0_AS(nicenv_t * e);
int wl0_EXE(nicenv_t * e);
int wl0_XEX(nicenv_t * e);

int wl0_NAME(nicenv_t * e);
int wl0_NOM(nicenv_t * e);
int wl0_LN(nicenv_t * e);
int wl0_CP(nicenv_t * e);
int wl0_DEL(nicenv_t * e);
int wl0_ADEL(nicenv_t * e);
int wl0_AVAR(nicenv_t * e);
int wl0_AUTO(nicenv_t * e);

int wl0_LOCK(nicenv_t * e);
int wl0_UNLO(nicenv_t * e);
int wl0_STRI(nicenv_t * e);
int wl0_PRIN(nicenv_t * e);
int wl0_PR(nicenv_t * e);
int wl0_VIEW(nicenv_t * e);
int wl0_SAVE(nicenv_t * e);
int wl0_APEN(nicenv_t * e);
int wl0_LOAD(nicenv_t * e);
int wl0_EDIT(nicenv_t * e);

int wl0_VOID(nicenv_t * e);
int wl0_UDEF(nicenv_t * e);
int wl0_BYTE(nicenv_t * e);
int wl0_LETR(nicenv_t * e);
int wl0_CHAR(nicenv_t * e);
int wl0_INT(nicenv_t * e);
int wl0_DUBL(nicenv_t * e);
int wl0_TEXT(nicenv_t * e);
int wl0_WORD(nicenv_t * e);
int wl0_SCRI(nicenv_t * e);
int wl0_CODE(nicenv_t * e);
int wl0_DB(nicenv_t * e);
int wl0_DOT(nicenv_t * e);
int wl0_VEC(nicenv_t * e);
int wl0_UVEC(nicenv_t * e);
int wl0_PDOT(nicenv_t * e);
int wl0_RVEC(nicenv_t * e);
int wl0_MAT(nicenv_t * e);
int wl0_MATX(nicenv_t * e);

int wl0_LEN(nicenv_t * e);
int wl0_GLUE(nicenv_t * e);
int wl0_PACK(nicenv_t * e);
int wl0_INS(nicenv_t * e);

int wl0_INC(nicenv_t * e);
int wl0_DEC(nicenv_t * e);

int wl0_EXP(nicenv_t * e);
int wl0_LOG(nicenv_t * e);
int wl0_ABS(nicenv_t * e);
int wl0_SIGN(nicenv_t * e);
int wl0_POW(nicenv_t * e);
int wl0_SQRT(nicenv_t * e);
int wl0_COS(nicenv_t * e);
int wl0_SIN(nicenv_t * e);
int wl0_TAN(nicenv_t * e);
int wl0_ACOS(nicenv_t * e);
int wl0_ASIN(nicenv_t * e);
int wl0_ATAN(nicenv_t * e);

int wl0_NMAX(nicenv_t * e);
int wl0_NMIN(nicenv_t * e);
int wl0_NAVE(nicenv_t * e);

int wl0_ARRA(nicenv_t * e);
int wl0_BARR(nicenv_t * e);
int wl0_PUT(nicenv_t * e);
int wl0_PUTN(nicenv_t * e);
int wl0_NEWT(nicenv_t * e);

int wl0_CC(nicenv_t * e);

int wl0_HEAD(nicenv_t * e);
int wl0_TAIL(nicenv_t * e);
int wl0_CUT(nicenv_t * e);
int wl0_SLEN(nicenv_t * e);
int wl0_OFFS(nicenv_t * e);
int wl0_SPLI(nicenv_t * e);
int wl0_CHOP(nicenv_t * e);
int wl0_CIN(nicenv_t * e);

int wl0_TICK(nicenv_t * e);
int wl0_USEC(nicenv_t * e);

int wl0_NTOA(nicenv_t * e);

int wl0_DADD(nicenv_t * e);
int wl0_DDEL(nicenv_t * e);
int wl0_DFIN(nicenv_t * e);

int wl0_WAIT(nicenv_t * e);
int wl0_EXIT(nicenv_t * e);

int wl0_INCH(nicenv_t * e);
int wl0_INMM(nicenv_t * e);

int wl0_UN(nicenv_t * e);

int wl0_SMUL(nicenv_t * e);
int wl0_VMUL(nicenv_t * e);
int wl0_VADD(nicenv_t * e);
int wl0_INV(nicenv_t * e);
int wl0_VSUB(nicenv_t * e);

int wl0_ANGL(nicenv_t * e);
int wl0_GRAD(nicenv_t * e);
int wl0_RAD(nicenv_t * e);

int wl0_LINE(nicenv_t * e);
int wl0_UMAT(nicenv_t * e);
int wl0_TMAT(nicenv_t * e);
int wl0_RXYZ(nicenv_t * e);
int wl0_MMUL(nicenv_t * e);
int wl0_MADD(nicenv_t * e);
int wl0_MTRA(nicenv_t * e);

int wl0_NEL(nicenv_t * e);
int wl0_PATH(nicenv_t * e);
int wl0_CLOS(nicenv_t * e);
int wl0_OPEN(nicenv_t * e);
int wl0_SIMP(nicenv_t * e);
int wl0_ARC(nicenv_t * e);

int wl0_STOK(nicenv_t * e);
int wl0_STOP(nicenv_t * e);
int wl0_GO(nicenv_t * e);

int wl0_RIPC(nicenv_t * e);

int wl0_HELP(nicenv_t * e);

int wl0_CMPS(nicenv_t * e);
int wl0_NOT(nicenv_t * e);

int wl0_ETON(nicenv_t * e);

int wl0_BIN(nicenv_t * e);
int wl0_HEX(nicenv_t * e);

#endif /*_WORDL0_H_*/