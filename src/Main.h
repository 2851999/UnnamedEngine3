
#include <iostream>

#include "core/BaseEngine.h"
#include "core/maths/Matrix.h"
#include "core/maths/Quaternion.h"
#include "utils/Logging.h"
#include "utils/TimeUtils.h"

class EngineTest : public BaseEngine {
public:
    virtual void initialise() override;
    virtual void created() override;
    virtual void update() override;
    virtual void render() override;
    virtual void destroy() override;

    void onKeyPressed(int key, bool repeated) override;
};

void EngineTest::initialise() {
    Logger::setLogLevel(LogType::Information | LogType::Warning | LogType::Error);

    std::cout << "Hello World" << std::endl;

    Vector<float, 2> testVec;
    testVec[0] = 1.0;
    testVec[1] = 1.0;

    std::cout << testVec.toString() << std::endl;
    testVec.normalise();
    std::cout << testVec.toString() << std::endl;

    Vector<float, 2> testVec2 = {1, 2};
    std::cout << testVec2.toString() << std::endl;

    Vector3<float> testVec3 = {1, 2, 3};
    std::cout << testVec3.toString() << std::endl;

    Vector2<float> testVec4 = Vector2<float>(testVec3);
    std::cout << testVec4.toString() << std::endl;

    Vector2f testVec5 = {1, 2};
    std::cout << testVec5.toString() << std::endl;

    Vector3i testVec6 = {1, 2, 3};
    Vector3i testVec7 = {4, 5, 6};
    Vector3i cross    = testVec6.cross(testVec7);

    std::cout << cross.toString() << std::endl;

    Matrix2f mat1 = {{1, 2},
                     {3, 4}};
    Matrix2f mat2;
    mat2.set(0, 0, 1);
    mat2.set(0, 1, 2);
    mat2.set(1, 0, 3);
    mat2.set(1, 1, 4);
    Vector2f res = mat2 * testVec5;
    std::cout << mat1.toString() << std::endl;
    std::cout << (mat1 * mat2).toString() << std::endl;
    mat1 *= mat2;
    std::cout << mat1.toString() << std::endl;
    std::cout << testVec5.toString() << std::endl;
    std::cout << mat2.toString() << std::endl;
    std::cout << res.toString() << std::endl;

    Matrix3f mat3 = {{1, 6, 3}, {4, 2, 6}, {-5, 8, 2}};
    std::cout << mat3.toString() << std::endl;
    Matrix3f inverse = mat3.inverse();
    std::cout << inverse.toString() << std::endl;

    Quaternion quat1  = {1, 2, 3, 4};
    Quaternion quat2  = {1, 2, 3, 4};
    Quaternion result = quat1 * quat2;

    std::cout << result.toString() << std::endl;
    quat1 *= quat2;
    std::cout << quat1.toString() << std::endl;

    // std::cout << utils_time::getTimeAsString() << std::endl;

    Logger::log("Test message");

    std::cout << utils_string::toInt("10") << std::endl;

    getSettings().video.maxFPS           = 60;
    getSettings().debug.validationLayers = true;
    getSettings().video.rayTracing       = true;
}

void EngineTest::created() {
}

void EngineTest::update() {
    // std::cout << getFPS() << std::endl;
}

void EngineTest::render() {
}

void EngineTest::destroy() {
}

void EngineTest::onKeyPressed(int key, bool repeated) {
    if (key == GLFW_KEY_ESCAPE)
        getWindow()->close();
}

int main() {
    EngineTest engineTest;
    engineTest.create();

    return 0;
}