/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------1------------------------------------------- */

/**
 * @file testExpression.cpp
 * @date September 18, 2014
 * @author Frank Dellaert
 * @author Paul Furgale
 * @brief unit tests for Block Automatic Differentiation
 */

#include <gtsam/nonlinear/Expression.h>
#include <gtsam/geometry/PinholeCamera.h>
#include <gtsam/geometry/Cal3_S2.h>
#include <gtsam/base/Testable.h>

#include <CppUnitLite/TestHarness.h>

#include <gtsam/linear/VectorValues.h>
#include <gtsam/nonlinear/ExpressionFactor.h>

#include <boost/assign/list_of.hpp>
using boost::assign::list_of;
using boost::assign::map_list_of;

using namespace std;
using namespace gtsam;

/* ************************************************************************* */
template<class CAL>
Point2 uncalibrate(const CAL& K, const Point2& p, OptionalJacobian<2, 5> Dcal,
    OptionalJacobian<2, 2> Dp) {
  return K.uncalibrate(p, Dcal, Dp);
}

static const Rot3 someR = Rot3::RzRyRx(1, 2, 3);

/* ************************************************************************* */
// Constant
TEST(Expression, Constant) {
  Expression<Rot3> R(someR);
  Values values;
  Rot3 actual = R.value(values);
  EXPECT(assert_equal(someR, actual));
  EXPECT_LONGS_EQUAL(0, R.traceSize())
}

/* ************************************************************************* */
// Leaf
TEST(Expression, Leaf) {
  Expression<Rot3> R(100);
  Values values;
  values.insert(100, someR);

  Rot3 actual2 = R.value(values);
  EXPECT(assert_equal(someR, actual2));
}

/* ************************************************************************* */
// Many Leaves
TEST(Expression, Leaves) {
  Values values;
  Point3 somePoint(1, 2, 3);
  values.insert(Symbol('p', 10), somePoint);
  std::vector<Expression<Point3> > points = createUnknowns<Point3>(10, 'p', 1);
  EXPECT(assert_equal(somePoint, points.back().value(values)));
}

/* ************************************************************************* */
// Unary(Leaf)
namespace unary {
Point2 f1(const Point3& p, OptionalJacobian<2, 3> H) {
  return Point2();
}
double f2(const Point3& p, OptionalJacobian<1, 3> H) {
  return 0.0;
}
Vector f3(const Point3& p, OptionalJacobian<Eigen::Dynamic, 3> H) {
  return p.vector();
}
Expression<Point3> p(1);
set<Key> expected = list_of(1);
}
TEST(Expression, Unary1) {
  using namespace unary;
  Expression<Point2> e(f1, p);
  EXPECT(expected == e.keys());
}
TEST(Expression, Unary2) {
  using namespace unary;
  Expression<double> e(f2, p);
  EXPECT(expected == e.keys());
}
/* ************************************************************************* */
// Unary(Leaf), dynamic
TEST(Expression, Unary3) {
  using namespace unary;
//  Expression<Vector> e(f3, p);
}
/* ************************************************************************* */
//Nullary Method
TEST(Expression, NullaryMethod) {

  // Create expression
  Expression<Point3> p(67);
  Expression<double> norm(p, &Point3::norm);

  // Create Values
  Values values;
  values.insert(67, Point3(3, 4, 5));

  // Check dims as map
  std::map<Key, int> map;
  norm.dims(map);
  LONGS_EQUAL(1, map.size());

  // Get value and Jacobians
  std::vector<Matrix> H(1);
  double actual = norm.value(values, H);

  // Check all
  EXPECT(actual == sqrt(50));
  Matrix expected(1, 3);
  expected << 3.0 / sqrt(50.0), 4.0 / sqrt(50.0), 5.0 / sqrt(50.0);
  EXPECT(assert_equal(expected, H[0]));
}
/* ************************************************************************* */
// Binary(Leaf,Leaf)
namespace binary {
// Create leaves
double doubleF(const Pose3& pose, //
    const Point3& point, OptionalJacobian<1, 6> H1, OptionalJacobian<1, 3> H2) {
  return 0.0;
}
Expression<Pose3> x(1);
Expression<Point3> p(2);
Expression<Point3> p_cam(x, &Pose3::transform_to, p);
}
/* ************************************************************************* */
// Check that creating an expression to double compiles
TEST(Expression, BinaryToDouble) {
  using namespace binary;
  Expression<double> p_cam(doubleF, x, p);
}
/* ************************************************************************* */
// keys
TEST(Expression, BinaryKeys) {
  set<Key> expected = list_of(1)(2);
  EXPECT(expected == binary::p_cam.keys());
}
/* ************************************************************************* */
// dimensions
TEST(Expression, BinaryDimensions) {
  map<Key, int> actual, expected = map_list_of<Key, int>(1, 6)(2, 3);
  binary::p_cam.dims(actual);
  EXPECT(actual == expected);
}
/* ************************************************************************* */
// dimensions
TEST(Expression, BinaryTraceSize) {
  typedef internal::BinaryExpression<Point3, Pose3, Point3> Binary;
  size_t expectedTraceSize = sizeof(Binary::Record);
  EXPECT_LONGS_EQUAL(expectedTraceSize, binary::p_cam.traceSize());
}
/* ************************************************************************* */
// Binary(Leaf,Unary(Binary(Leaf,Leaf)))
namespace tree {
using namespace binary;
// Create leaves
Expression<Cal3_S2> K(3);

// Create expression tree
Point2 (*f)(const Point3&, OptionalJacobian<2, 3>) = &PinholeBase::Project;
Expression<Point2> projection(f, p_cam);
Expression<Point2> uv_hat(uncalibrate<Cal3_S2>, K, projection);
}
/* ************************************************************************* */
// keys
TEST(Expression, TreeKeys) {
  set<Key> expected = list_of(1)(2)(3);
  EXPECT(expected == tree::uv_hat.keys());
}
/* ************************************************************************* */
// dimensions
TEST(Expression, TreeDimensions) {
  map<Key, int> actual, expected = map_list_of<Key, int>(1, 6)(2, 3)(3, 5);
  tree::uv_hat.dims(actual);
  EXPECT(actual == expected);
}
/* ************************************************************************* */
// TraceSize
TEST(Expression, TreeTraceSize) {
  typedef internal::BinaryExpression<Point3, Pose3, Point3> Binary1;
  EXPECT_LONGS_EQUAL(internal::upAligned(sizeof(Binary1::Record)),
      tree::p_cam.traceSize());

  typedef internal::UnaryExpression<Point2, Point3> Unary;
  EXPECT_LONGS_EQUAL(
      internal::upAligned(sizeof(Unary::Record)) + tree::p_cam.traceSize(),
      tree::projection.traceSize());

  EXPECT_LONGS_EQUAL(0, tree::K.traceSize());

  typedef internal::BinaryExpression<Point2, Cal3_S2, Point2> Binary2;
  EXPECT_LONGS_EQUAL(
      internal::upAligned(sizeof(Binary2::Record)) + tree::K.traceSize()
          + tree::projection.traceSize(), tree::uv_hat.traceSize());
}
/* ************************************************************************* */

TEST(Expression, compose1) {

  // Create expression
  Expression<Rot3> R1(1), R2(2);
  Expression<Rot3> R3 = R1 * R2;

  // Check keys
  set<Key> expected = list_of(1)(2);
  EXPECT(expected == R3.keys());
}

/* ************************************************************************* */
// Test compose with arguments referring to the same rotation
TEST(Expression, compose2) {

  // Create expression
  Expression<Rot3> R1(1), R2(1);
  Expression<Rot3> R3 = R1 * R2;

  // Check keys
  set<Key> expected = list_of(1);
  EXPECT(expected == R3.keys());
}

/* ************************************************************************* */
// Test compose with one arguments referring to constant rotation
TEST(Expression, compose3) {

  // Create expression
  Expression<Rot3> R1(Rot3::identity()), R2(3);
  Expression<Rot3> R3 = R1 * R2;

  // Check keys
  set<Key> expected = list_of(3);
  EXPECT(expected == R3.keys());
}

/* ************************************************************************* */
// Test with ternary function
Rot3 composeThree(const Rot3& R1, const Rot3& R2, const Rot3& R3,
    OptionalJacobian<3, 3> H1, OptionalJacobian<3, 3> H2,
    OptionalJacobian<3, 3> H3) {
  // return dummy derivatives (not correct, but that's ok for testing here)
  if (H1)
    *H1 = eye(3);
  if (H2)
    *H2 = eye(3);
  if (H3)
    *H3 = eye(3);
  return R1 * (R2 * R3);
}

TEST(Expression, ternary) {

  // Create expression
  Expression<Rot3> A(1), B(2), C(3);
  Expression<Rot3> ABC(composeThree, A, B, C);

  // Check keys
  set<Key> expected = list_of(1)(2)(3);
  EXPECT(expected == ABC.keys());
}

/* ************************************************************************* */
// Test with multiple compositions on duplicate keys

bool checkMatricesNear(const Eigen::MatrixXd& lhs, const Eigen::MatrixXd& rhs,
                       double tolerance) {
  bool near = true;
  for (int i = 0; i < lhs.rows(); ++i) {
    for (int j = 0; j < lhs.cols(); ++j) {
      const double& lij = lhs(i, j);
      const double& rij = rhs(i, j);
      const double& diff = std::abs(lij - rij);
      if (!std::isfinite(lij) ||
          !std::isfinite(rij) ||
          diff > tolerance) {
        near = false;

        std::cout << "\nposition " << i << "," << j << " evaluates to "
            << lij << " and " << rij << std::endl;
      }
    }
  }
  return near;
}


// Compute finite difference Jacobians for an expression factor.
template<typename T>
JacobianFactor computeFiniteDifferenceJacobians(ExpressionFactor<T> expression_factor,
                                                const Values& values,
                                                double fd_step) {
  Eigen::VectorXd e = expression_factor.unwhitenedError(values);
  const size_t rows = e.size();

  std::map<Key, Eigen::MatrixXd> jacobians;
  typename ExpressionFactor<T>::const_iterator key_it = expression_factor.begin();
  VectorValues dX = values.zeroVectors();
  for(; key_it != expression_factor.end(); ++key_it) {
    size_t key = *key_it;
    // Compute central differences using the values struct.
    const size_t cols = dX.dim(key);
    Eigen::MatrixXd J = Eigen::MatrixXd::Zero(rows, cols);
    for(size_t col = 0; col < cols; ++col) {
      Eigen::VectorXd dx = Eigen::VectorXd::Zero(cols);
      dx[col] = fd_step;
      dX[key] = dx;
      Values eval_values = values.retract(dX);
      Eigen::VectorXd left = expression_factor.unwhitenedError(eval_values);
      dx[col] = -fd_step;
      dX[key] = dx;
      eval_values = values.retract(dX);
      Eigen::VectorXd right = expression_factor.unwhitenedError(eval_values);
      J.col(col) = (left - right) * (1.0/(2.0 * fd_step));
    }
    jacobians[key] = J;
  }

  // Next step...return JacobianFactor
  return JacobianFactor(jacobians, -e);
}

template<typename T>
bool testExpressionJacobians(Expression<T> expression,
                             const Values& values,
                             double fd_step,
                             double tolerance) {
  // Create factor
  size_t size = traits<T>::dimension;
  ExpressionFactor<T> f(noiseModel::Unit::Create(size), expression.value(values), expression);

  // Check linearization
  JacobianFactor expected = computeFiniteDifferenceJacobians(f, values, fd_step);
  boost::shared_ptr<GaussianFactor> gf = f.linearize(values);
  boost::shared_ptr<JacobianFactor> jf = //
      boost::dynamic_pointer_cast<JacobianFactor>(gf);

  typedef std::pair<Eigen::MatrixXd, Eigen::VectorXd> Jacobian;
  Jacobian evalJ = jf->jacobianUnweighted();
  Jacobian estJ = expected.jacobianUnweighted();

  return checkMatricesNear(evalJ.first, estJ.first, tolerance);
}

double doubleSumImplementation(const double& v1, const double& v2,
                               OptionalJacobian<1, 1> H1, OptionalJacobian<1, 1> H2) {
  if (H1) {
    H1->setIdentity();
  }
  if (H2) {
    H2->setIdentity();
  }
  return v1+v2;
}

TEST(Expression, testMultipleCompositions) {
  const double tolerance = 1e-5;
  const double fd_step = 1e-9;

  double v1 = 0;
  double v2 = 1;

  Values values;
  values.insert(1, v1);
  values.insert(2, v2);

  Expression<double> ev1(Key(1));
  Expression<double> ev2(Key(2));

  Expression<double> sum(doubleSumImplementation, ev1, ev2);
  Expression<double> sum2(doubleSumImplementation, sum, ev1);
  Expression<double> sum3(doubleSumImplementation, sum2, sum);

  std::cout << "Testing sum " << std::endl;
  EXPECT(testExpressionJacobians(sum, values, fd_step, tolerance));
  std::cout << "Testing sum2 " << std::endl;
  EXPECT(testExpressionJacobians(sum2, values, fd_step, tolerance));
  std::cout << "Testing sum3 " << std::endl;
  EXPECT(testExpressionJacobians(sum3, values, fd_step, tolerance));
}

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */

