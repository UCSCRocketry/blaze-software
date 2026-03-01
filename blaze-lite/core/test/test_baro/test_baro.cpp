#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <unity.h>

void test_baro_functionality() {
    TEST_ASSERT_EQUAL(1, 1);
}

#ifdef ARDUINO
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_baro_functionality);
    UNITY_END();
}

void loop() {
}
#else
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_baro_functionality);
    return UNITY_END();
}
#endif