
only_corr=-sens_countour_draw 0 -ref_countour_draw 0 -sens_points_draw 0 -ref_points_draw 0 -sens_rays_draw 0 -ref_rays_draw 0 -write_info 1

all:
	json_extract -nth 0 -in stallo2.log -out stallo2-0.json
	json_extract -nth 1 -in stallo2.log -out stallo2-1.json
	log2pdf -config hokuyo.config -use odometry -in stallo2-0.json
	log2pdf -config hokuyo.config -use odometry -in stallo2-1.json
	log2pdf -config hokuyo.config -use odometry -in stallo2.log -distance_xy 0
	sm2 -restart 0 -in stallo2.log -out stallo2_plicp.log -max_iterations 10 -file_jj stallo2_plicp.journal
	sm_animate $(only_corr) -in  stallo2_plicp.journal -out 'stallo2_plicp_%02d.pdf'
	sm2 -restart 0  -in stallo2.log -out stallo2_icp.log  -use_point_to_line_distance  0 -max_iterations 10 -file_jj stallo2_icp.journal
	sm_animate $(only_corr) -in  stallo2_icp.journal -out 'stallo2_icp_%02d.pdf'
	sm2    -in stallo2.log -out stallo2_gpm.log  -algo 1 -file_jj stallo2_gpm.journal
	log2pdf -config hokuyo.config -use estimate -in stallo2_gpm.log
	sm2  -restart 0 -max_correspondence_dist  0.2 -in stallo2.log -out stallo2_plicp2.log -max_iterations 10 -file_jj stallo2_plicp2.journal
	sm_animate $(only_corr) -in  stallo2_plicp2.journal -out 'stallo2_plicp2_%02d.pdf'
	sm2  -restart 0 -max_correspondence_dist  0.2 -in stallo2.log -out stallo2_icp2.log -max_iterations 10 -file_jj stallo2_icp2.journal  -use_point_to_line_distance  0
	sm_animate $(only_corr) -in  stallo2_icp2.journal -out 'stallo2_icp2_%02d.pdf'

