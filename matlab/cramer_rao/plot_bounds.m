function f = plot_bound(res)

s = size(res.etmin,2);

if false
res.etmax(1,1) = 0;
res.etmax(size(res.etmin,1)-3,size(res.etmin,2)-3) = 0;
res.etmax(1,size(res.etmin,2)-10) = 0;
end

f= figure
subplot(1,4,1)
imagesc(good_direction(res.etmax))
AXIS('image');
AXIS('off');
title('\sigma_1 = max \sigma Cov(t)')


subplot(1,4,2)
imagesc(good_direction(res.etmin))
AXIS('image');
AXIS('off');
title('\sigma_2 =  min \sigma Cov(t)')

subplot(1,4,3)
imagesc(good_direction(sqrt(res.etmin.*res.etmax)))
AXIS('image');
AXIS('off');
title('sqrt(\sigma_1 \sigma_2)')

subplot(1,4,4)
imagesc(res.eth')
AXIS('image');
AXIS('off');
title('var(\phi)');

f = figure
imagesc(good_direction((res.eth.*res.etmin.*res.etmax).^(1/3)))
AXIS('image');
AXIS('off');
title('det');


function res = good_direction(x)
	for i=1:size(x,1)
	for j=1:size(x,2)	
		res(i,j) = x(j,size(x,2)-i+1);
		% res(i,j) = x(i,j);
	end
	end
