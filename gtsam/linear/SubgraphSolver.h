///* ----------------------------------------------------------------------------
//
// * GTSAM Copyright 2010, Georgia Tech Research Corporation,
// * Atlanta, Georgia 30332-0415
// * All Rights Reserved
// * Authors: Frank Dellaert, et al. (see THANKS for the full author list)
//
// * See LICENSE for the license information
//
// * -------------------------------------------------------------------------- */
//
//#pragma once
//
//#include <boost/make_shared.hpp>
//
//#include <gtsam/linear/GaussianFactorGraph.h>
//#include <gtsam/linear/IterativeSolver.h>
//#include <gtsam/linear/SubgraphPreconditioner.h>
//
//namespace gtsam {
//
///* split the gaussian factor graph Ab into Ab1 and Ab2 according to the map */
//bool split(const std::map<Index, Index> &M,
//		const GaussianFactorGraph &Ab,
//		GaussianFactorGraph &Ab1,
//		GaussianFactorGraph &Ab2);
//
///**
// * A nonlinear system solver using subgraph preconditioning conjugate gradient
// * Concept NonLinearSolver<G,T,L> implements
// *   linearize: G * T -> L
// *   solve : L -> VectorValues
// */
//template<class GRAPH, class LINEAR, class VALUES>
//class SubgraphSolver : public IterativeSolver {
//
//private:
//	typedef typename VALUES::Key Key;
//	typedef typename GRAPH::Pose Pose;
//	typedef typename GRAPH::Constraint Constraint;
//
//	typedef boost::shared_ptr<const SubgraphSolver> shared_ptr ;
//	typedef boost::shared_ptr<Ordering> shared_ordering ;
//	typedef boost::shared_ptr<GRAPH> shared_graph ;
//	typedef boost::shared_ptr<LINEAR> shared_linear ;
//	typedef boost::shared_ptr<VALUES> shared_values ;
//	typedef boost::shared_ptr<SubgraphPreconditioner> shared_preconditioner ;
//	typedef std::map<Index,Index> mapPairIndex ;
//
//	/* the ordering derived from the spanning tree */
//	shared_ordering ordering_;
//
//	/* the indice of two vertices in the gaussian factor graph */
//	mapPairIndex pairs_;
//
//	/* preconditioner */
//	shared_preconditioner pc_;
//
//	/* flag for direct solver - either QR or LDL */
//	bool useQR_;
//
//public:
//
//	SubgraphSolver(const GRAPH& G, const VALUES& theta0, const Parameters &parameters = Parameters(), bool useQR = false):
//		IterativeSolver(parameters), useQR_(useQR) { initialize(G,theta0); }
//
//	SubgraphSolver(const LINEAR& GFG) {
//		std::cout << "[SubgraphSolver] Unexpected usage.." << std::endl;
//		throw std::runtime_error("SubgraphSolver: gaussian factor graph initialization not supported");
//	}
//
//	SubgraphSolver(const shared_linear& GFG, const boost::shared_ptr<VariableIndex>& structure, bool useQR = false) {
//		std::cout << "[SubgraphSolver] Unexpected usage.." << std::endl;
//		throw std::runtime_error("SubgraphSolver: gaussian factor graph and variable index initialization not supported");
//	}
//
//	SubgraphSolver(const SubgraphSolver& solver) :
//		IterativeSolver(solver), ordering_(solver.ordering_), pairs_(solver.pairs_), pc_(solver.pc_), useQR_(solver.useQR_) {}
//
//	SubgraphSolver(shared_ordering ordering,
//			mapPairIndex pairs,
//			shared_preconditioner pc,
//			sharedParameters parameters = boost::make_shared<Parameters>(),
//			bool useQR = true) :
//				IterativeSolver(parameters), ordering_(ordering), pairs_(pairs), pc_(pc), useQR_(useQR) {}
//
//	void replaceFactors(const typename LINEAR::shared_ptr &graph);
//	VectorValues::shared_ptr optimize() ;
//	shared_ordering ordering() const { return ordering_; }
//
//protected:
//	void initialize(const GRAPH& G, const VALUES& theta0);
//
//private:
//	SubgraphSolver():IterativeSolver(){}
//};
//
//} // namespace gtsam
//
//#include <gtsam/linear/SubgraphSolver-inl.h>
