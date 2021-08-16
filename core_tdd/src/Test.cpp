#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

#include "sim_inc.h"
#include "sim_hex.h"
#include <time.h>

#include "mcu_tdd.h"
#include "m0pi_tdd.h"
#include "coreNOP.h"
#include "coreRSBS.h"
#include "coreCMN.h"
#include "coreMULSTest.h"
#include "coreMVNS.h"
#include "coreADD_T2Test.h"
#include "coreCMP_T2Test.h"
#include "coreSTRTest.h"
#include "coreLDR_T1Test.h"
#include "coreXTTest.h"
#include "coreBTest.h"
#include "coreCPSTest.h"
#include "coreREVTest.h"
#include "coreBKPTTest.h"
#include "coreSUBTest.h"
#include "simGpioTest.h"


bool runAllTests(int argc, char const *argv[]) {
	cute::suite s { };

  // core instructions test
  s.push_back(CUTE(coreNOPTest));
  s.push_back(CUTE(coreRSBSTest));
  s.push_back(CUTE(coreCMNTest));
  s.push_back(CUTE(coreMULSTest));
  s.push_back(CUTE(coreMVNSTest));
  s.push_back(CUTE(coreADD_T2Test));
  s.push_back(CUTE(coreCMP_T2Test));
  s.push_back(CUTE(coreSTRH_T1Test));
  s.push_back(CUTE(coreSTRB_T1Test));
  s.push_back(CUTE(coreLDRSB_T1Test));
  s.push_back(CUTE(coreLDRH_T1Test));
  s.push_back(CUTE(coreLDRSH_T1Test));
  s.push_back(CUTE(coreSTRH_im_T1Test));
  s.push_back(CUTE(coreLDRH_im_T1Test));
  s.push_back(CUTE(coreADR_T1Test));
  s.push_back(CUTE(coreADDS_imm3_T1Test));
  s.push_back(CUTE(coreADDS_imm8_T2Test));
  s.push_back(CUTE(coreADCS_T1Test));
  s.push_back(CUTE(coreSXTHTest));
  s.push_back(CUTE(coreSXTBTest));
  s.push_back(CUTE(coreUXTHTest));
  s.push_back(CUTE(coreUDFT1Test));
  s.push_back(CUTE(coreUDFT2Test));
  s.push_back(CUTE(coreSVCTest));
  s.push_back(CUTE(coreCPSTest));
  s.push_back(CUTE(coreREVTest));
  s.push_back(CUTE(coreREV16Test));
  s.push_back(CUTE(coreREVSHTest));
  s.push_back(CUTE(coreBKPTTest));
  s.push_back(CUTE(coreWFETest));
  s.push_back(CUTE(coreWFITest));
  s.push_back(CUTE(coreSEVTest));
  s.push_back(CUTE(coreSUB_imm3_T1Test));
  s.push_back(CUTE(coreSUB_imm8_T2Test));
  s.push_back(CUTE(coreSUB_reg_T1Test));
  s.push_back(CUTE(coreSBC_reg_T1Test));
  s.push_back(CUTE(coreCMP_reg_T1Test));
  s.push_back(CUTE(coreCMN_reg_T1Test));

  s.push_back(CUTE(mcuCreateAndInitTest));

  s.push_back(CUTE(simGpioTest));

  cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<>> lis(xmlfile.out);
	auto runner = cute::makeRunner(lis, argc, argv);
	bool success = runner(s, "AllTests");
	return success;
}

int main(int argc, char const *argv[]) {
    return runAllTests(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}

//ASSERTM(msg, cond)
//ASSERT(cond)
//ASSERT_EQUALM(msg, expected, actual)
//ASSERT_EQUAL(expected, actual)
//ASSERT_EQUAL_DELTAM(msg, expected, actual, delta)
//ASSERT_EQUAL_DELTA(expected, actual, delta)
//ASSERT_EQUAL_RANGES(expbeg, expend, actbeg, actend)
//ASSERT_EQUAL_RANGESM(msg, expbeg, expend, actbeg, actend)
//ASSERT_GREATERM(msg, left, right)
//ASSERT_GREATER(left, right)
//ASSERT_GREATER_EQUALM(msg, left, right)
//ASSERT_GREATER_EQUAL(left, right)
//ASSERT_LESSM(msg, left, right)
//ASSERT_LESS(left, right)
//ASSERT_LESS_EQUAL(left, right)
//ASSERT_LESS_EQUALM(msg, left, right)
//ASSERT_THROWS(code, exc)
//ASSERT_THROWSM(msg, code, exc)
//FAIL()
//FAILM(msg)
//ASSERT*_DDT(cond, failure)
//ASSERT*_DDTM(msg, cond, failure)
//ASSERT_NOT_EQUAL_TO(left, right)
//ASSERT_NOT_EQUAL_TOM(msg, left, right)
