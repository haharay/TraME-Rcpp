/*
 * Generic Linear/Quadratic Programming solver using Gurobi
 * Based on Gurobi's 'dense_c.c' example
 *
 * Keith O'Hara
 * 05/08/2016
 *
 * clang -O2 -Wall -I/opt/local/include -I/Library/gurobi650/mac64/include generic_lp.c -c -o generic_lp.o
 * gcc-mp-5 -O2 -Wall -I/opt/local/include -I/Library/gurobi650/mac64/include generic_lp.c -c -o generic_lp.o
 */
 
#include <string.h>

#include "generic_lp.h"

int generic_LP_C(int rows, int cols, double* obj, double* A, int modelSense, double* rhs, char* sense, double* Q, double* lb, double* ub, double* objval, double* sol_mat_X, double* sol_mat_RC, double* dual_mat_PI, double* dual_mat_SLACK)
{
    int i, j, optimstatus;
    int error = 0;
    int success = 0;
    
    /* Create an empty model */
    
    GRBenv* env = NULL;
    
    error = GRBloadenv(&env, NULL);
    if (error) goto QUIT;
    
    GRBmodel *model = NULL;
    
    error = GRBnewmodel(env, &model, "trame_LP", cols, obj, lb, ub, NULL, NULL);
    if (error) goto QUIT;
    
    error = GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_OUTPUTFLAG, 0);
    if (error) goto QUIT;
    
    if(modelSense==0){ // minimize or maximize, resp.
        error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, 1);
    }else{
        error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, -1);
    }
    if (error) goto QUIT;

    error = GRBaddconstrs(model, rows, 0, NULL, NULL, NULL, sense, rhs, NULL);
    if (error) goto QUIT;
    
    /* Integrate new rows and columns */
    
    error = GRBupdatemodel(model);
    if (error) goto QUIT;
    
    /* Populate A matrix */
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (A[i*cols+j] != 0) {
                error = GRBchgcoeffs(model, 1, &i, &j, &A[i*cols+j]);
                if (error) goto QUIT;
            }
        }
    }
    
    /* Populate Q matrix */

    if (Q) {
        for (i = 0; i < cols; i++) {
            for (j = 0; j < cols; j++) {
                if (Q[i*cols+j] != 0) {
                    error = GRBaddqpterms(model, 1, &i, &j, &Q[i*cols+j]);
                    if (error) goto QUIT;
                }
            }
        }
    }
    
    error = GRBupdatemodel(model);
    if (error) goto QUIT;

    /* Optimize model */

    error = GRBoptimize(model);
    if (error) goto QUIT;

    /* Capture solution information */

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) goto QUIT;

    if (optimstatus == GRB_OPTIMAL) {

        error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, objval);
        if (error) goto QUIT;

        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, cols, sol_mat_X);
        if (error) goto QUIT;
        
        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_RC, 0, cols, sol_mat_RC);
        if (error) goto QUIT;
        
        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_PI, 0, rows, dual_mat_PI);
        if (error) goto QUIT;
        
        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_SLACK, 0, rows, dual_mat_SLACK);
        if (error) goto QUIT;

        success = 1;
    }

QUIT:

    /* Error reporting */

    if (error) {
        printf("ERROR: %s\n", GRBgeterrormsg(env));
        exit(1);
    }

    /* Free model */

    GRBfreemodel(model);
    GRBfreeenv(env);

    return success;
}

int generic_LP_C_switch(int rows, int cols, double* obj, double* A, int modelSense, double* rhs, char* sense, double* Q, double* lb, double* ub, double* objval, double* sol_mat_X, double* sol_mat_RC, double* dual_mat_PI, double* dual_mat_SLACK)
{
    int i, j, optimstatus;
    int error = 0;
    int success = 0;
    int numremoved = 0;
    //printf("Begin!\n");
    /* Create an empty model */
    
    GRBenv* env = NULL;
    
    error = GRBloadenv(&env, NULL);
    if (error) goto QUIT;
    
    GRBmodel *model = NULL;
    
    error = GRBnewmodel(env, &model, "trame_LP", cols, obj, lb, ub, NULL, NULL);
    if (error) goto QUIT;
    
    error = GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_OUTPUTFLAG, 0);
    if (error) goto QUIT;
    
    if(modelSense==0){ // minimize or maximize, resp.
        error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, 1);
    }else{
        error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, -1);
    }
    if (error) goto QUIT;

    error = GRBaddconstrs(model, rows, 0, NULL, NULL, NULL, sense, rhs, NULL);
    if (error) goto QUIT;
    
    /* Integrate new rows and columns */
    
    error = GRBupdatemodel(model);
    if (error) goto QUIT;
    
    /* Populate A matrix */
    
    /* IMPORTANT: when using the generic_lp.hpp version, we switch
     * from:
     *
     * error = GRBchgcoeffs(model, 1, &i, &j, &A[i*rows+j]);
     *
     * to something like &A[i+j*cols] because of a weird issue where
     * the reference would run over rows rather than columns first
     */
    
    //int act_row = 0;
    //int act_col = 0;
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            /*if (A[i*cols+j] != 0) {
                error = GRBchgcoeffs(model, 1, &act_row, &act_col, &A[i*cols+j]);
                if (error) goto QUIT;
            }*/
            if (A[i+j*rows] != 0) {
                error = GRBchgcoeffs(model, 1, &i, &j, &A[i+j*rows]);
                if (error) goto QUIT;
            }
            
            /*++act_row;
            if(act_row>=rows){
                act_row=0;
                ++act_col;
            }*/
        }
    }
    
    /* Populate Q matrix */

    /* IMPORTANT: when using the generic_lp.hpp version, we switch
     * from:
     *
     * error = GRBaddqpterms(model, 1, &i, &j, &Q[i*cols+j]);
     *
     * to something like &Q[i+j*cols] because of a weird issue where
     * the reference would run over rows rather than columns first
     */

    if (Q) {
        //act_row = 0;
        //act_col = 0;
        
        for (i = 0; i < cols; i++) {
            for (j = 0; j < cols; j++) {
                /*if (Q[i*cols+j] != 0) {
                    error = GRBaddqpterms(model, 1, &act_row, &act_col, &Q[i*cols+j]);
                    if (error) goto QUIT;
                }*/
                
                if (Q[i+j*cols] != 0) {
                    error = GRBaddqpterms(model, 1, &i, &j, &Q[i+j*cols]);
                    if (error) goto QUIT;
                }
                
                /*++act_row;
                if(act_row>=cols){
                    act_row=0;
                    ++act_col;
                }*/
            }
        }
    }
    
    error = GRBupdatemodel(model);
    if (error) goto QUIT;
    //printf("Setup Model!\n");
    /* Optimize model */
    
    //error = GRBsetintparam(env, "Method", 1);
    //if (error) goto QUIT;

    error = GRBoptimize(model);
    if (error) goto QUIT;

    /* Capture solution information */

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) goto QUIT;
    
    //printf("optim code: %d\n", optimstatus);

    if (optimstatus==4) { // INF_OR_UNBD
        printf("Optim status: 4 (INF_OR_UNBD). Reoptimizing... ");
        
        error = GRBsetintparam(GRBgetenv(model), GRB_INT_PAR_DUALREDUCTIONS, 0);
        if (error) goto QUIT;
        
        error = GRBupdatemodel(model);
        if (error) goto QUIT;
    
        error = GRBoptimize(model);
        if (error) goto QUIT;
        
        error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
        if (error) goto QUIT;
        
        printf("New optim code: %d\n", optimstatus);
        
        if (optimstatus == GRB_UNBOUNDED) {
            printf("The model cannot be solved because it is unbounded\n");
            goto QUIT;
        }
    }

    if (optimstatus==3) { // infeasible model; reduce constraints
        int iis, numconstrs;
        char*  cname;
        int*   cind = NULL;
        char** removed = NULL;
        
        cind = malloc(sizeof(int) * rows * cols);
        if (!cind) goto QUIT;
        
        printf("Optim status: 3 (Infeasible Model). Reoptimizing by removing constraints (IIS)... \n");
        
        error = GRBcomputeIIS(model);
        if (error) goto QUIT;
        
        /* Loop until we reduce to a model that can be solved */
        error = GRBgetintattr(model, "NumConstrs", &numconstrs);
        if (error) goto QUIT;
  
        removed = calloc(numconstrs, sizeof(char*));
        if (!removed) goto QUIT;
        
        while (1) {
            error = GRBcomputeIIS(model);
            if (error) goto QUIT;
            
            //printf("\nThe following constraint cannot be satisfied:\n");
            
            for (i = 0; i < numconstrs; ++i) {
                error = GRBgetintattrelement(model, "IISConstr", i, &iis);
                if (error) goto QUIT;
                
                if (iis) {
                    error = GRBgetstrattrelement(model, "ConstrName", i, &cname);
                    if (error) goto QUIT;
                    
                    //printf("%s\n", cname);
                    
                    /* Remove a single constraint from the model */
                    removed[numremoved] = malloc(sizeof(char) * (1+strlen(cname)));
                    if (!removed[numremoved]) goto QUIT;
                    
                    strcpy(removed[numremoved++], cname);
                    
                    cind[0] = i;
                    
                    error = GRBdelconstrs(model, 1, cind);
                    if (error) goto QUIT;
                    
                    break;
                }
            }
            
            //printf("\n");
            
            error = GRBupdatemodel(model);
            if (error) goto QUIT;
            
            error = GRBoptimize(model);
            if (error) goto QUIT;
            
            error = GRBgetintattr(model, "Status", &optimstatus);
            if (error) goto QUIT;
            
            if (optimstatus == GRB_UNBOUNDED) {
                printf("The model cannot be solved because it is unbounded\n");
                goto QUIT;
            }
            
            if (optimstatus == GRB_OPTIMAL) {
                break;
            }
            
            if ((optimstatus != GRB_INF_OR_UNBD) && (optimstatus != GRB_INFEASIBLE)) {
                printf("Optimization was stopped with status %i\n", optimstatus);
                goto QUIT;
            }
        }
        //
        printf("New optim code: %d. Number of constraints removed: %d\n", optimstatus, numremoved);
    }


    if (optimstatus == GRB_OPTIMAL) {

        error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, objval);
        if (error) goto QUIT;

        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, cols, sol_mat_X);
        if (error) goto QUIT;
        
        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_RC, 0, cols, sol_mat_RC);
        if (error) goto QUIT;
        
        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_PI, 0, rows-numremoved, dual_mat_PI);
        if (error) goto QUIT;
        
        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_SLACK, 0, rows-numremoved, dual_mat_SLACK);
        if (error) goto QUIT;

        success = 1;
    }

    //printf("Finished! Success: %d\n", success);

    /* Free model */

    GRBfreemodel(model);
    GRBfreeenv(env);

    return success;

QUIT:

    /* Error reporting */

    if (error) {
        printf("ERROR: %s\n", GRBgeterrormsg(env));
        exit(1);
    }

    /* Free model */

    GRBfreemodel(model);
    GRBfreeenv(env);

    return success;
}