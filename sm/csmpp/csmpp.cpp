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

sm_result CanonicalScanMatcher::scanMatch(LDP refScan, LDP laserScan)
{

    params.setLaserRef(refScan);
    params.setLaserSen(laserScan);
    double odometry[3];
    double ominus_laser[3], temp[3];    
    pose_diff_d(laserScan->odometry, refScan->odometry, odometry);
    ominus_d(params.getParams()->laser, ominus_laser);
    oplus_d(ominus_laser, odometry, temp);
    oplus_d(temp, params.getParams()->laser, params.getParams()->first_guess);

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
        return matchingResult;
    }
    oplus_d(refScan->estimate, matchingResult.x, laserScan->estimate);
    return matchingResult;
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
