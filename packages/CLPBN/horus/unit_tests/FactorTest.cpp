#include <iostream>

#include "../Factor.h"
#include "Common.h"


namespace Horus {

namespace UnitTests {

class FactorTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE (FactorTest);
    CPPUNIT_TEST (testSummingOut);
    CPPUNIT_TEST (testProduct);
    CPPUNIT_TEST_SUITE_END();
  public:
    void testSummingOut();
    void testProduct();
};



void
FactorTest::testSummingOut()
{
  VarIds vids   = {0, 1, 2, 3};
  Ranges ranges = {3, 2, 4, 3};
  Params params = {
    0.022757933283133, 0.0106825417145475, 0.0212115929862968, 
    0.0216271252738214, 0.0246935408909929, 0.00535101952882101, 
    0.00908008645423061, 0.0208088234425334, 0.00752400708452212, 
    0.0150052316136527, 0.0129311224551535, 0.0170340535302049, 
    0.00988081654256193, 0.0139063490493519, 0.025792784294836, 
    0.0248167234610076, 0.017219348482278, 0.0194292243637016, 
    0.00383554941557795, 0.0164407987747966, 0.00044152909395022, 
    0.00657900705816833, 0.00371715392294919, 0.0217825142487465, 
    0.00424392333677727, 0.0108602703755316, 0.00351559808401304, 
    0.00294727405145356, 0.0270575932871257, 0.005911864680038, 
    0.0138936584911577, 0.0227288019859002, 0.0165944064071987, 
    0.0080185268930961, 0.0172692026753632, 0.0142012227138332, 
    0.0133695464219171, 0.0263492891422071, 0.00792332157200822, 
    0.0208935535064392, 0.0142677961715013, 0.0208544440271617, 
    0.0108408824522857, 0.0241486127140633, 0.00767406849215521, 
    0.00954694217537661, 0.0218786116033257, 0.0248934169744332, 
    0.00188944195471982, 0.0257141610189036, 0.0142474911774847, 
    0.00233097104867004, 0.00520644350532678, 0.0179646451004339, 
    0.0241134853100298, 0.00945036684210405, 0.00173819089160705, 
    0.000542358809684406, 0.0123976408935576, 0.00170905959437435, 
    0.00645422348972241, 0.0262912993847153, 0.0244378615928878, 
    0.0230486298969212, 0.00722310170606624, 0.0146203396838926, 
    0.0101631280263959, 0.0205926481279833, 0.0138829042417413, 
    0.0180864495984042, 0.0143994770626774, 0.00106397584149748
  };

  Factor f (vids, ranges, params);
  double sum = std::accumulate (f.params().begin(), f.params().end(), 0.0);
  CPPUNIT_ASSERT (similiar (sum, 1.0));
  
  f.sumOut (0);
  f.sumOut (3);
  f.sumOut (2);
  
  sum = std::accumulate (f.params().begin(), f.params().end(), 0.0);
  CPPUNIT_ASSERT (similiar (sum, 1.0));
}



void
FactorTest::testProduct()
{
  VarIds vids1   = {0, 1, 2};
  Ranges ranges1 = {3, 2, 2};
  Params params1 = {
    0.01, 0.02, 0.03, 0.04, 0.05, 0.06,
    0.07, 0.08, 0.09, 0.10, 0.11, 0.12
  };

  VarIds vids2   = {1, 3, 0};
  Ranges ranges2 = {2, 3, 3};
  Params params2 = {
    0.15, 0.30, 0.45, 0.60, 0.75, 0.90, 1.20, 1.50, 1.80,
    0.99, 0.88, 0.77, 0.66, 0.55, 0.44, 0.33, 0.22, 0.11
  };

  Factor f1 (vids1, ranges1, params1);
  Factor f2 (vids2, ranges2, params2);
  f1.multiply (f2);

  Params result = {
    0.0015, 0.006, 0.012, 0.003, 0.012, 0.024, 0.0297, 0.0198, 0.0099,
    0.0396, 0.0264, 0.0132, 0.015, 0.0375, 0.075, 0.018, 0.045, 0.09,
    0.0616, 0.0385, 0.0154, 0.0704, 0.044, 0.0176, 0.0405, 0.081, 0.162,
    0.045, 0.09, 0.18, 0.0847, 0.0484, 0.0121,  0.0924, 0.0528, 0.0132
  };

  CPPUNIT_ASSERT (similiar (f1.params(), result));
}



CPPUNIT_TEST_SUITE_REGISTRATION (FactorTest);

}  // namespace UnitTests

}  // namespace Horus
