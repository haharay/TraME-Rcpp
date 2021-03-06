/*################################################################################
  ##
  ##   Copyright (C) 2015 - 2018 the TraME Team:
  ##      Alfred Galichon
  ##      Keith O'Hara
  ##
  ##   This file is part of TraME.
  ##
  ##   TraME is free software: you can redistribute it and/or modify
  ##   it under the terms of the GNU General Public License as published by
  ##   the Free Software Foundation, either version 2 of the License, or
  ##   (at your option) any later version.
  ##
  ##   TraME is distributed in the hope that it will be useful,
  ##   but WITHOUT ANY WARRANTY; without even the implied warranty of
  ##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  ##   GNU General Public License for more details.
  ##
  ##   You should have received a copy of the GNU General Public License
  ##   along with TraME. If not, see <http://www.gnu.org/licenses/>.
  ##
  ################################################################################*/

/*
 * Cupids LP solver
 *
 * Keith O'Hara
 * 09/15/2017
 */

#include "R_solvers.hpp"

RCPP_EXPOSED_CLASS(dse_empirical_tu_R)

SEXP cupids_lp_R(SEXP market_inp)
{
    try {
        bool success = false;
        double val_out;
        arma::mat mu_out, U_out, V_out;
        arma::vec mu_x0_out, mu_0y_out;

        if (Rf_inherits(market_inp, "Rcpp_dse_empirical_tu")) {
            dse_empirical_tu_R* market_obj = Rcpp::as<dse_empirical_tu_R*>(market_inp);

            success = trame::cupids_lp(*market_obj,mu_out,mu_x0_out,mu_0y_out,U_out,V_out,val_out);
        }
        else {
            ::Rf_error( "trame: unrecognized market type" );
            return R_NilValue;
        }

        return Rcpp::List::create(Rcpp::Named("mu") = mu_out, Rcpp::Named("mu_x0") = mu_x0_out, Rcpp::Named("mu_0y") = mu_0y_out, 
                                  Rcpp::Named("U") = U_out, Rcpp::Named("V") = V_out, Rcpp::Named("val") = val_out, Rcpp::Named("success") = success);
    } catch( std::exception &ex ) {
        forward_exception_to_r( ex );
    } catch(...) {
        ::Rf_error( "trame: C++ exception (unknown reason)" );
    }
    return R_NilValue;
}
