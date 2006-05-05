function f = plot_bound(res)

prefix = 'tbs'


s = size(res.etmin,2);

if false
res.eth(1,1) = 0;
res.eth(size(res.etmin,1)-3,size(res.etmin,2)-3) = 0;
res.eth(1,size(res.etmin,2)-10) = 0;
end

f= figure
subplot(1,5,1)
etmax = good_direction(res.etmax);
writeToFile(etmax,strcat(prefix, '_max.png'));
imagesc(etmax)
AXIS('image');
AXIS('off');
title('\sigma_1 = max \sigma Cov(t)')


subplot(1,5,2)
etmin=good_direction(res.etmin);
writeToFile(etmax,strcat(prefix, '_min.png'));
imagesc(etmin)
AXIS('image');
AXIS('off');
title('\sigma_2 =  min \sigma Cov(t)')

subplot(1,5,3)
mea = good_direction(sqrt(res.etmin.*res.etmax));
imagesc(mea)
writeToFile(mea,strcat(prefix, '_mean.png'));
AXIS('image');
AXIS('off');
title('sqrt(\sigma_1 \sigma_2)')

subplot(1,5,4)
eth = good_direction(sqrt(res.eth));
imagesc(eth)
writeToFile(eth,strcat(prefix, '_th.png'));
AXIS('image');
AXIS('off');
title('var(\phi)');


tot = good_direction((res.eth.*res.etmin.*res.etmax).^(1/6));
subplot(1,5,5)
imagesc(tot)
AXIS('image');
AXIS('off');
title('det');

print(gcf,'-dpng', 'bounds.png')
%colormap('prism')

f = figure
imagesc(tot)
writeToFile(tot,strcat(prefix, '_tot.png'));
AXIS('image');
AXIS('off');
title('det');

%colormap('pink')

function writeToFile(image, file)
 
%map = colormap
%ind2rgb(mat2gray(etmax)*255,map)
%imwrite(, 

function res = good_direction(x)
	for i=1:size(x,1)
	for j=1:size(x,2)	
		res(i,j) = real(x(j,size(x,2)-i+1));
		% res(i,j) = x(i,j);
	end
	end
