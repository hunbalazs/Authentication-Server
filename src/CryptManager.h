// ToDo
// Understand something lol

#ifndef TR_CRYPT_MANAGER_H
#define TR_CRYPT_MANAGER_H

#ifdef _WIN32
#include "windows.h"
#else
#include <cstring>
#endif
#include <string>
#include <mutex>
#include "MD5.h"

#define N 16

// Blowfish struct
struct BLOWFISH_CTX 
{
    uint64_t P[16 + 2];
    uint64_t S[4][256];
};


// TR structs
struct _DecStruct1T
{
	uint8_t D1[6];
};

struct _DecStruct2T
{
	uint8_t D1[64];
};

struct _DecStruct3T
{
	uint8_t D1[1024];
};

class CryptManager
{
	// Blowfish data
	static const uint8_t	sdata[];
	static const uint8_t	BF_PTransformed[4*18];
	static const uint64_t	ORIG_P[16 + 2];
	static const uint64_t	ORIG_S[4][256];

	// TR Data
	static const uint8_t	DecArray1[0x38];
	static const uint8_t	DecArray2_CEA3D0[0x10];
	static const uint8_t	DecArray3[0x10];
	static const uint8_t	DecArray4_CEA180[48];
	static const uint8_t	DecArray5_CEA3B0[32];
	static const uint32_t 	DecArray6_CEA3F0[4];
	static const uint8_t	CEA1B0_DATA[16*64];
	static uint8_t			DecArrayOut1_D23548[0x38];
	static uint8_t			DecArrayOut2_D1E4B0[0x38];
	static uint8_t			DecArrayOut3_D1ECE8[4096*4];
	static const uint8_t	InputData_0CEA0B8[0x40];
	static const uint8_t	InputData_0CEA0F8[0x40];
	static uint8_t			OutputData_D1E4E8[128*16];
	static uint8_t			OutputData_D22D48[128*16];

	public:
		static CryptManager* Instance();
		static CryptManager* Create();
		~CryptManager();

		// MD5 function
		std::string GenMD5(char* data, uint32_t length);

		// Blowfish functions
		void BFInit();
		void BFDecrypt(uint64_t *xl, uint64_t *xr);
		void BFEncrypt(uint64_t *xl, uint64_t *xr);

		// TR functions
		void TRInit();
		void TREncrypt(uint8_t *Data, uint32_t Len);
		void TRDecrypt(uint8_t *Data, uint32_t Len);

        void lock();
        void unlock();
		
	protected:
		CryptManager();

	private:
		static CryptManager* Pointer;

        std::mutex  TRC_mutex;

		// Blowfish structs
		BLOWFISH_CTX BlowfishContext;

		// Blowfish functions
		uint64_t F(uint64_t x);

		// TR structs
		_DecStruct1T	DecStruct1[16];
		_DecStruct2T   *DecStruct2_CEA1B0;
		_DecStruct3T	DecStruct3_D1D4B0[4];

		// TR functions
		void            TRPrepareBasic(uint8_t *Output, const uint8_t *Input);
		void            TRKeyIntegrate(uint8_t *Key);
		void            TRKeyIntegrate2();
		void            TRKeyIntegrate3();

		int32_t 		sub_A7D470(int32_t a1, int32_t a2);
		void 			sub_A7D8D0_3(const uint8_t *DataP, uint8_t *B_, uint8_t *Out);
		void	 		sub_A7D790(uint8_t *a1, uint8_t *a2);
		void 			sub_A7D4B0(uint8_t *a1, uint8_t *a2);
		void 			sub_A7D5E0_5(uint8_t *p1, uint8_t *p2);
		void	 		sub_A7DA60_4(uint8_t *d, int32_t idx, uint8_t *a3);
		void 			sub_A7DC90_3(int32_t idx, uint8_t *m, uint8_t *m2);
		void			sub_A7DE00_2(uint8_t *DataP1, uint8_t *DataP2);
		void 			sub_A7DFD0(uint8_t *a1, uint8_t *a2);
		void 			sub_A7E190_1(uint8_t *Data, uint32_t Len, bool State);
};

#endif
