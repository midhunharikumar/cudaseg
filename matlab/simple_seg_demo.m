%region_seg(I,init_mask,max_its,E,T)
%coded by hormuz mostofi

I = imread('squares.bmp');               %-- load the image
m = zeros(size(I,1),size(I,2));          %-- create initial mask

I = imresize(I,.3);
m = imresize(m,.3);

subplot(2,2,1); imshow(I); title('Input Image');
subplot(2,2,2);
[x,y]=getline('closed');
m=poly2mask(x,y,size(I,1),size(I,2));
imshow(m); title('Initial Mask'); hold on; contour(m); hold off;


seg = simpleseg(I, m, 400, 40, 240); %-- Run segmentationm, set last parameter = 205

subplot(2,2,1); imshow(seg); title('Final Mask of phi<=0');