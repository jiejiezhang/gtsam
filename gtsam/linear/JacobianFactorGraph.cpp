/**
 * @file   JacobianFactorGraph.h
 * @date   Jun 6, 2012
 * @Author Yong Dian Jian
 */

#include <gtsam/linear/JacobianFactorGraph.h>
#include <boost/foreach.hpp>

namespace gtsam {

/* ************************************************************************* */
Errors operator*(const FactorGraph<JacobianFactor>& fg, const VectorValues& x) {
  Errors e;
  BOOST_FOREACH(const JacobianFactor::shared_ptr& Ai, fg) {
    e.push_back((*Ai)*x);
  }
  return e;
}

/* ************************************************************************* */
void multiplyInPlace(const FactorGraph<JacobianFactor>& fg, const VectorValues& x, Errors& e) {
  multiplyInPlace(fg,x,e.begin());
}

/* ************************************************************************* */
void multiplyInPlace(const FactorGraph<JacobianFactor>& fg, const VectorValues& x, const Errors::iterator& e) {
  Errors::iterator ei = e;
  BOOST_FOREACH(const JacobianFactor::shared_ptr& Ai, fg) {
    *ei = (*Ai)*x;
    ei++;
  }
}


/* ************************************************************************* */
// x += alpha*A'*e
void transposeMultiplyAdd(const FactorGraph<JacobianFactor>& fg, double alpha, const Errors& e, VectorValues& x) {
  // For each factor add the gradient contribution
  Errors::const_iterator ei = e.begin();
  BOOST_FOREACH(const JacobianFactor::shared_ptr& Ai, fg) {
    Ai->transposeMultiplyAdd(alpha,*(ei++),x);
  }
}

/* ************************************************************************* */
VectorValues gradient(const FactorGraph<JacobianFactor>& fg, const VectorValues& x0) {
  // It is crucial for performance to make a zero-valued clone of x
  VectorValues g = VectorValues::Zero(x0);
  Errors e;
  BOOST_FOREACH(const JacobianFactor::shared_ptr& factor, fg) {
    e.push_back(factor->error_vector(x0));
  }
  transposeMultiplyAdd(fg, 1.0, e, g);
  return g;
}

/* ************************************************************************* */
void gradientAtZero(const FactorGraph<JacobianFactor>& fg, VectorValues& g) {
  // Zero-out the gradient
  g.setZero();
  Errors e;
  BOOST_FOREACH(const JacobianFactor::shared_ptr& factor, fg) {
    e.push_back(-factor->getb());
  }
  transposeMultiplyAdd(fg, 1.0, e, g);
}

/* ************************************************************************* */
void residual(const FactorGraph<JacobianFactor>& fg, const VectorValues &x, VectorValues &r) {
  Index i = 0 ;
  BOOST_FOREACH(const JacobianFactor::shared_ptr& factor, fg) {
    r[i] = factor->getb();
    i++;
  }
  VectorValues Ax = VectorValues::SameStructure(r);
  multiply(fg,x,Ax);
  axpy(-1.0,Ax,r);
}

/* ************************************************************************* */
void multiply(const FactorGraph<JacobianFactor>& fg, const VectorValues &x, VectorValues &r) {
  r.vector() = Vector::Zero(r.dim());
  Index i = 0;
  BOOST_FOREACH(const JacobianFactor::shared_ptr& factor, fg) {
    SubVector &y = r[i];
    for(JacobianFactor::const_iterator j = factor->begin(); j != factor->end(); ++j) {
      y += factor->getA(j) * x[*j];
    }
    ++i;
  }
}

/* ************************************************************************* */
void transposeMultiply(const FactorGraph<JacobianFactor>& fg, const VectorValues &r, VectorValues &x) {
  x.vector() = Vector::Zero(x.dim());
  Index i = 0;
  BOOST_FOREACH(const JacobianFactor::shared_ptr& factor, fg) {
    for(JacobianFactor::const_iterator j = factor->begin(); j != factor->end(); ++j) {
      x[*j] += factor->getA(j).transpose() * r[i];
    }
    ++i;
  }
}
}


