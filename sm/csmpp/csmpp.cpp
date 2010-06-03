#include "csmpp.h"

CanonicalScanMatcher::CanonicalScanMatcher(SMParameters parameters):
        params(parameters),
        showDebug(0),
        recoverFromError(0),
        matchingAlgorithm(ICP)
{

}

CanonicalScanMatcher::CanonicalScanMatcher():
        showDebug(0),
        recoverFromError(0),
        matchingAlgorithm(ICP)
{
}

CanonicalScanMatcher::~CanonicalScanMatcher()
{
}

bool CanonicalScanMatcher::scanMatch(LDP refScan, LDP secondScan)
{

    params.setLaserRef(refScan);
    params.setLaserSen(secondScan);

    switch(matchingAlgorithm)
    {
    case(ICP):
        sm_icp(params.getParams(), &matchingResult);
        break;
    case(GPM):
        sm_gpm(params.getParams(), &matchingResult);
        break;
    case(HSM):
        sm_hsm(params.getParams(), &matchingResult);
        break;
    default:
        sm_error("Unknown algorithm to run: %d.\n",matchingAlgorithm);
        return false;
    }

    return true;
}

void CanonicalScanMatcher::setShowDebug(bool state)
{
    this->showDebug = state;
}

void CanonicalScanMatcher::setSMParameters(SMParameters parameters)
{
    this->params = parameters;
}

void CanonicalScanMatcher::setRecoverFromError(bool state)
{
    this->recoverFromError = state;
}
