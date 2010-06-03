#ifndef CSM_WRAPPER
#define CSM_WRAPPER

#include <iostream>
#include <csm/csm_all.h>

class SMParameters
{
public:
    SMParameters()
    {
        // Default Values
        this->smParameters.max_angular_correction_deg          = 90;
        this->smParameters.max_linear_correction               = 2;
        this->smParameters.max_iterations                      = 1000;
        this->smParameters.epsilon_xy                          = 0.0001;
        this->smParameters.epsilon_theta                       = 0.0001;
        this->smParameters.max_correspondence_dist             = 2;
        this->smParameters.sigma                               = 0.01;
        this->smParameters.use_corr_tricks                     = 1;
        this->smParameters.restart                             = 1;
        this->smParameters.restart_threshold_mean_error        = 0.01;
        this->smParameters.restart_dt                          = 0.01;
        this->smParameters.restart_dtheta                      = 0.0261799;
        this->smParameters.clustering_threshold                = 0.05;
        this->smParameters.orientation_neighbourhood           = 3;
        this->smParameters.use_point_to_line_distance          = 1;
        this->smParameters.do_alpha_test                       = 0;
        this->smParameters.do_alpha_test_thresholdDeg          = 20;
        this->smParameters.outliers_maxPerc                    = 0.95;
        this->smParameters.outliers_adaptive_order             = 0.7;
        this->smParameters.outliers_adaptive_mult              = 2;
        this->smParameters.do_visibility_test                  = 0;
        this->smParameters.outliers_remove_doubles             = 1;
        this->smParameters.do_compute_covariance               = 0;
        this->smParameters.debug_verify_tricks                 = 0;
        this->smParameters.gpm_theta_bin_size_deg              = 5;
        this->smParameters.gpm_extend_range_deg                = 15;
        this->smParameters.gpm_interval                        = 1;
        this->smParameters.min_reading                         = 0;
        this->smParameters.max_reading                         = 1000;
        this->smParameters.use_ml_weights                      = 0;
        this->smParameters.use_sigma_weights                   = 0;
        this->smParameters.hsm.linear_cell_size                = 0.03;
        this->smParameters.hsm.angular_cell_size_deg           = 1;
        this->smParameters.hsm.num_angular_hypotheses          = 8;
        this->smParameters.hsm.xc_directions_min_distance_deg  = 10;
        this->smParameters.hsm.xc_ndirections                  = 3;
        this->smParameters.hsm.angular_hyp_min_distance_deg    = 10;
        this->smParameters.hsm.linear_xc_max_npeaks            = 5;
        this->smParameters.hsm.linear_xc_peaks_min_distance    = 5;
    }
    ~SMParameters(){}
    sm_params * getParams()
    {
        return & this->smParameters;
    }

    /*! Where to start */
    void setfirstGuess(double firtGuess[3])
    {
        this->smParameters.first_guess[0]  = firtGuess[0];
        this->smParameters.first_guess[1]  = firtGuess[1];
        this->smParameters.first_guess[2]  = firtGuess[2];
    }

    void setLaserRef(LDP laserRef)
    {
        this->smParameters.laser_ref = laserRef;
    }

    void setLaserSen(LDP laserSen)
    {
        this->smParameters.laser_sens = laserSen;
    }

    /*! Maximum angular displacement between scans (deg)*/
    void setMaxAngularCorrectionDeg(double maxAng)
    {
        this->smParameters.max_angular_correction_deg  = maxAng;
    }
    /*! Maximum translation between scans (m) */
    void setMaxLinearCorrection(double maxLinear)
    {
        this->smParameters.max_linear_correction  = maxLinear ;
    }
    /*! When to stop */
    void setMaxIterations(int maxIt)
    {
        this->smParameters.max_iterations  = maxIt;
    }
    /*! A threshold for stopping. */
    void setEpsilonXY(double eps)
    {
        this->smParameters.epsilon_xy  = eps;
    }
    /*! A threshold for stopping. */
    void setEpsilonTheta(double epsTheta)
    {
        this->smParameters.epsilon_theta  = epsTheta;
    }
    /*! Maximum distance for a correspondence to be valid */
    void setMaxCorrespondenceDist(double maxCorrDist)
    {
        this->smParameters.max_correspondence_dist  = maxCorrDist ;
    }
    /*! Use smart tricks for finding correspondences. Only influences speed; not convergence. */
    void setUseCorrTricks(bool useTricks)
    {
        this->smParameters.use_corr_tricks  = useTricks;
    }
    /*! Restart if error under threshold (0 or 1)*/
    void setRestart(bool restart)
    {
        this->smParameters.restart  = restart;
    }
    /*! Threshold for restarting */
    void setRestartThresholdMeanError(double restartME)
    {
        this->smParameters.restart_threshold_mean_error  = restartME;
    }
    /*! Displacement for restarting */
    void setRestartDT(double restardt)
    {
        this->smParameters.restart_dt  = restardt;
    }
    /*! Displacement for restarting */
    void setRestartDTheta(double restartdtTheta)
    {
        this->smParameters.restart_dtheta  = restartdtTheta;
    }
    /*
        Functions concerning discarding correspondences.
        THESE ARE MAGIC NUMBERS -- and they need to be tuned.
     */
    /*!
        Percentage of correspondences to consider: if 0.9,
        always discard the top 10% of correspondences with more error
     */
    void setOutliersMaxPerc(double maxPerc)
    {
        this->smParameters.outliers_maxPerc  = maxPerc;
    }
    /*!
        Parameters describing a simple adaptive algorithm for discarding.
            1) Order the errors.
            2) Choose the percentile according to outliers_adaptive_order.
               (if it is 0.7, get the 70% percentile)
            3) Define an adaptive threshold multiplying outliers_adaptive_mult
               with the value of the error at the chosen percentile.
            4) Discard correspondences over the threshold.

            This is useful to be conservative; yet remove the biggest errors.
    */
    void setOutliersAdaptiveOrder(double adaptiveOrder)  /* 0.7 */
    {
        this->smParameters.outliers_adaptive_order  = adaptiveOrder;
    }
    void setOutliersAdaptiveMult(double adaptiveMult) /* 2 */
    {
        this->smParameters.outliers_adaptive_mult  = adaptiveMult;
    }
    /*! Do not allow two different correspondences to share a point */
    void setOutliersRemoveDoubles(bool removeDoubles)
    {
        this->smParameters.outliers_remove_doubles  = removeDoubles ;
    }
    /* Functions that compute and use point orientation for defining matches. */
    /*! For now, a very simple max-distance clustering algorithm is used */
    void setClusteringThreshold(double clusteringThresh)
    {
        this->smParameters.clustering_threshold  =  clusteringThresh;
    }
    /*! Number of neighbour rays used to estimate the orientation.*/
    void setOrientationNeighbourhood(int orientationNeighbourhood)
    {
        this->smParameters.orientation_neighbourhood  = orientationNeighbourhood ;
    }
    /*! Discard correspondences based on the angles */
    void setDoAlphaTest(bool doAlphaTest)
    {
        this->smParameters.do_alpha_test  = doAlphaTest ;
    }
    void setDoAlphaTestThresholdDeg(int alphaTestThresh)
    {
        this->smParameters.do_alpha_test_thresholdDeg  = alphaTestThresh;
    }
    void setDoVisibilityTest(bool doVisibility )
    {
        this->smParameters.do_visibility_test  = doVisibility ;
    }
    /*! If 1, use PlICP; if 0, use vanilla ICP. */
    void setUsePoint2LineDistance(bool usePoint2Line)
    {
        this->smParameters.use_point_to_line_distance  = usePoint2Line ;
    }
    /*!
        If 1, the field "true_alpha" is used to compute the incidence
        beta, and the factor (1/cos^2(beta)) used to weight the impact
        of each correspondence. This works fabolously if doing localization,
        that is the first scan has no noise.
        If "true_alpha" is not available, it uses "alpha".
    */
    void setUseMLWeights(bool useMLWeights)
    {
        this->smParameters.use_ml_weights  = useMLWeights ;
    }
    /*! If 1, the field "readings_sigma" is used to weight the correspondence by 1/sigma^2 */
    void setUseSigmaWeights(bool useSigmaWeights)
    {
        this->smParameters.use_sigma_weights  = useSigmaWeights ;
    }
    /*!
        Use the method in http://purl.org/censi/2006/icpcov to compute
        the matching covariance.
    */
    void setDoComputeCovariance(bool computeCovariance)
    {
        this->smParameters.do_compute_covariance  = computeCovariance ;
    }
    /*! Checks that find_correspondences_tricks give the right answer */
    void setDebugVerifyTricks(bool debugVerifyTricks)
    {
        this->smParameters.debug_verify_tricks  = debugVerifyTricks ;
    }
    /*! Pose of sensor with respect to robot: used for computing
        the first estimate given the odometry. */
    void setLaserPose(double laserPose[3])
    {
        this->smParameters.laser[0]  = laserPose[0] ;
        this->smParameters.laser[1]  = laserPose[1] ;
        this->smParameters.laser[2]  = laserPose[2] ;
    }
    /*! Noise in the scan */
    void setSigma(double sigma)
    {
        this->smParameters.sigma  = sigma;
    }
    /*! mark as invalid ( = don't use ) rays outside of this interval */
    void setMinReading(double minReading)
    {
        this->smParameters.min_reading  = minReading;
    }
    void setMaxReading(double maxReading)
    {
        this->smParameters.max_reading  = maxReading;
    }
    /*! Parameters specific to GPM (unfinished :-/ ) */
    void setGpmThetaBinSizeDeg(double gpmBinSize)
    {
        this->smParameters.gpm_theta_bin_size_deg  = gpmBinSize ;
    }
    void setGpmExtendRangeDeg(double extendRange)
    {
        this->smParameters.gpm_extend_range_deg  = extendRange ;
    }
    void setGpmInterval(int interval)
    {
        this->smParameters.gpm_interval  =  interval;
    }
    /*! Parameter specific to HSM (unfinished :-/ ) */
    void setHSM (struct hsm_params  hsmParams)
    {
        this->smParameters.hsm  = hsmParams;
    }
private:
    sm_params smParameters;
};

class CanonicalScanMatcher
{
    enum MatchingAlgorithims
    {
        ICP,
        GPM,
        HSM
    };
public:
    CanonicalScanMatcher();
    CanonicalScanMatcher(SMParameters parameters);
    ~CanonicalScanMatcher();
    void setSMParameters(SMParameters parameters);
    void setShowDebug(bool);
    void setRecoverFromError(bool);
    bool scanMatch(LDP refScan, LDP secondScan);
private:
    int matchingAlgorithm;
    bool showDebug;
    bool recoverFromError;
    SMParameters params;
    sm_result matchingResult;
};

#endif
