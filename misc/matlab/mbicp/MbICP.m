% Test implementation of Minguez et al Metric Based ICP
% by Patric Jensfelt, 2006-03-28
%
% refScan - 361 range readings from reference scan
% newScan - 361 range readings from reference scan
% initQ   - initial estimate of transformation from ref to new scan
% 
function transf = MbICP(refScan, newScan, initQ, interactive)

  global ang;
  global cx;
  global cy;
  global assoc;
  global n_assoc;
  
  % Parameters
  global L;
  global Max_Dist;
  global Min_error;

  % Values of the parameters
  L = 3.0;
  Max_Dist = 1;
  Min_error = 1e-4;
  
  nrays = length(refScan)
  %ang = (0:0.5:180)*pi/180;
  ang = (1:nrays)*(pi/nrays);
  ang = (1:nrays)*(pi/nrays)-pi/2;
  
  if nargin < 3
    disp('Usgae: MbICP(refScan, newScan, initQ)');
    return
  end

  if checkArgs(refScan, newScan, initQ)
    return
  end

  px = refScan .* cos(ang);
  py = refScan .* sin(ang);

  % Define array to hold Cartesian coordinates for transformed new scan
  cx = zeros(1,nrays);
  cy = zeros(1,nrays);

  % The transformation from the reference scan to the new scan
  q = initQ;

  for k = 1:500
    disp(sprintf('Iteration %d', k))

    calcTransfNewScanPts(newScan, q);
    clf, hold on
    dispScan(px,py,'r')
    dispScan(cx,cy,'b')
    drawnow

	if interactive
	    disp('Get associations')
	end
	
	n_assoc = 0;
    getAssociations(px,py,cx,cy);

	 if interactive
  	  dispAssociations(px,py,cx,cy,assoc,n_assoc);
      	disp('Getting best transformation')
	 end
	 
    q_min = getQmin(px,py,cx,cy,assoc, n_assoc)

    q = compound(q_min, q)

    if (max(abs(q_min)) < Min_error) 
      disp(sprintf('Converged in iteration %d', k))
      break
    end
  
	 if interactive
    disp(sprintf('Press enter for iteration %d', k+1))
    pause
	 end
  end

  dispAssociations(px,py,cx,cy,assoc,n_assoc);
  disp('Final transformation:');
  q

  transf=q;
  
  return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function err = checkArgs(refScan, newScan, initQ)
  
	err = 0
	
	if length(refScan) < 10
    disp('Need at least 10 ranges for ref scan');
    err = 1;
    return;
  end

  if  not(length(newScan) == length(refScan)) 
    disp('Need ref scan and new scan of same size');
    err = 1;
    return;
  end

  if length(initQ) < 3
    disp('Need 3 values for initial transformation');
    err = 1;
    return;
  end

  return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function calcTransfNewScanPts(newScan, q)
  global cx;
  global cy;
  global ang;

  R = [cos(q(3)) -sin(q(3)); sin(q(3)) cos(q(3))];
  xy = R * [newScan .* cos(ang); newScan .* sin(ang)];
  cx = xy(1,:) + q(1);
  cy = xy(2,:) + q(2);

  return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function dispScan(x,y,col)
  plot(x,y,[col '.'])
return

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function getAssociations(px,py,cx,cy)
  global assoc
  global n_assoc
  global Max_Dist

  for ic = 1:length(cx)
    
    minD2 = 1e10;
    minIndex = -1;

    for ip = 1:length(px)

      d2 = getMbDistSqr(px(ip),py(ip),cx(ic),cy(ic));
      if d2 < minD2
        minD2 = d2;
        minIndex = ip;
      end
    end

    % Add this association (not very clever to reallocate every time...)
    if minD2 < Max_Dist,
      n_assoc = n_assoc + 1;
      assoc(n_assoc,1) = minIndex;
      assoc(n_assoc,2) = ic;
    end

  end

  return

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function d2 = getMbDistSqr(p2x, p2y, p1x, p1y)
  global L;

  dx = p2x - p1x;
  dy = p2y - p1y;

  tmp = dx * p1y - dy * p1x;
  d2 = dx*dx + dy*dy - (tmp * tmp / (p1y*p1y + p1x*p1x + L*L));

  return

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function dispAssociations(px,py,cx,cy,assoc, n_assoc)

  for k=1:n_assoc
     plot([px(assoc(k,1)) cx(assoc(k,2))], [py(assoc(k,1)) cy(assoc(k,2))])
  end

  return

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function q_min = getQmin(px,py,cx,cy,assoc, n_assoc)

  global L;

  A = zeros(3,3);
  b = zeros(3,1);
  
  for k = 1:n_assoc
    pix = px(assoc(k,1));
    piy = py(assoc(k,1));
    cix = cx(assoc(k,2));
    ciy = cy(assoc(k,2));

    ki = pix * pix + piy * piy + L * L;

    cxpxPcypy = cix * pix + ciy * piy;
    cxpyMcypx = cix * piy - ciy * pix;

    A(1,1) = A(1,1) + 1.0 - piy * piy / ki;
    A(1,2) = A(1,2) + pix * piy / ki;
    A(1,3) = A(1,3) - ciy + piy / ki * cxpxPcypy; 	
    A(2,2) = A(2,2) + 1.0 - pix * pix / ki; 			
    A(2,3) = A(2,3) + cix - pix / ki * cxpxPcypy;
    A(3,3) = A(3,3) + cix*cix + ciy*ciy - cxpxPcypy*cxpxPcypy / ki;

    b(1,1) = b(1,1) + cix - pix - piy / ki * cxpyMcypx;
    b(2,1) = b(2,1) + ciy - piy + pix / ki * cxpyMcypx;
    b(3,1) = b(3,1) + (cxpxPcypy / ki - 1.0) * cxpyMcypx;
  end

  % Complete the A-matrix by assigning the symmetric portions of it
  A(2,1) = A(1,2);
  A(3,1) = A(1,3);
  A(3,2) = A(2,3);

  q_min = - inv(A) * b; 


  return

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function t_ret = compound(t1, t2)

  t_ret = zeros(3,1);

  t_ret(1) = t2(1) * cos(t1(3)) - t2(2) * sin(t1(3)) + t1(1);
  t_ret(2) = t2(1) * sin(t1(3)) + t2(2) * cos(t1(3)) + t1(2);
  t_ret(3) = t1(3) + t2(3);

  % Make angle [-pi,pi)
  while (t_ret(3) >= pi)
    t_ret(3) = t_ret(3) - 2*pi;
  end
  while (t_ret(3) < -pi)
    t_ret(3) = t_ret(3) + 2*pi;
  end

  return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function t_ret = inv_compound(t)

  t_ret = zeros(3,1);

  t_ret(1) = -t(1) * cos(t(3)) - t(2) * sin(t(3));
  t_ret(2) =  t(1) * sin(t(3)) - t(3) * cos(t(3));
  t_ret(3) = -t(3);

  return


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

