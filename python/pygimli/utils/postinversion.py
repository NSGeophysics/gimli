#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Thu Feb 02 14:15:13 2012

@author: Guenther.T
"""

import pygimli as pg
from pygimli.utils import gmat2numpy
import numpy as np
# is there a way using numpy.linalg instead to avoid default scipy dependency?
#import scipy.linalg


def iterateBounds(inv, dchi2=0.5, maxiter=100, change=1.02):
    """
    Find parameter bounds by iterating model parameter until error
    bound is reached

    Parameters
    ----------
    inv - gimli inversion object
    dchi2 - allowed variation of chi^2 values [0.5]
    maxiter - maximum iteration number for parameter iteration [100]
    change - changing factor of parameters [1.02, i.e. 2%]
    """
    f = inv.forwardOperator()

    model = inv.model()
    resp = inv.response()

    nd, nm = len(resp), len(model)
    modelU = np.zeros(nm)
    modelL = np.zeros(nm)
    maxchi2 = inv.chi2() + dchi2

    for im in range(nm):
        model1 = pg.RVector(model)
        chi2, iter = 0., 0

        while (chi2 < maxchi2) & (iter < maxiter):
            iter += 1
            model1[im] *= change
            resp1 = f(model1)
            chi2 = inv.getPhiD(resp1) / nd

        modelU[im] = model1[im]

        model2 = pg.RVector(model)
        chi2, iter = 0., 0

        while (chi2 < maxchi2) & (iter < maxiter):
            iter += 1
            model2[im] /= change
            resp2 = f(model2)
            chi2 = inv.getPhiD(resp2) / nd

        modelL[im] = model2[im]

    return modelL, modelU


def modCovar(inv):
    """formal model covariance matrix (MCM) from inversion

    var, MCMs = modCovar(inv)

    Parameters
    ----------
    inv : pygimli inversion object

    Returns
    -------
    var  : variances (inverse square roots of MCM matrix)
    MCMs : scaled MCM (such that diagonals are 1.0)

    Example
    -------
    import pygimli as pg
    import matplotlib.pyplot as plt
    from matplotlib.cm import bwr
    INV = pg.RInversion(data, f)
    par = INV.run()
    var, MCM = modCovar(INV)
    i = plt.imshow(MCM, interpolation='nearest', cmap=bwr, vmin=-1, vmax=1)
    plt.colorbar(i)
    """
    td = np.asarray(inv.transData().deriv(inv.response()))
    tm = np.asarray(inv.transModel().deriv(inv.model()))

    J = td.reshape(len(td), 1) * \
        gmat2numpy(inv.forwardOperator().jacobian()) * (1. / tm)
    d = 1. / np.asarray(inv.transData().error(inv.response(), inv.error()))

    DJ = d.reshape(len(d), 1) * J
    JTJ = DJ.T.dot(DJ)
    try:
        MCM = scipy.linalg.inv(JTJ)   # model covariance matrix

        varVG = np.sqrt(np.diag(MCM))  # standard deviations from main diagonal
        di = (1.0 / varVG)  # variances as column vector

        # scaled model covariance (=correlation) matrix
        MCMs = di.reshape(len(di), 1) * MCM * di
        return varVG, MCMs
    except Exception as e:
        print(e)
        import traceback
        import sys

        traceback.print_exc(file=sys.stdout)
        return np.zeros(len(inv.model()),), 0


def print1dBlockVar(var, thk, xpos=None):
    """ does not belong here as it is a plotting function
    """
    raise('fixme')
    # welches plt??
    # if xpos is None:
    #xpos = plt.xlim()[0]

    #nlay = len(thk) + 1
    #zl  = np.cumsum(thk)
    #zvec = np.hstack((zl,zl-thk/2,zl[-1]+thk[-1]/2))

    # for j in range(nlay*2-1):
    #v = np.log(1.+var[j])
    # if j<nlay-1:
    #plt.text(xpos ,zvec[j],'$\delta$='+str(np.round_(v*thk[j], 1))+'m')
    # else:
    #plt.text(xpos, zvec[j],'$\delta$='+str(np.round_(v*100., 1))+'$\%$')
