%simple_seg(I,init_mask,max_its,E,T,alpha)
%coded by hormuz mostofi

I = imread('liver.bmp');               %-- load the image
m = zeros(size(I,1),size(I,2));          %-- create initial mask
I = imresize(I,0.6);
m = imresize(m,0.6);

subplot(2,2,1); imshow(I); title('Input Image');
subplot(2,2,2);
imshow(m); title('Initial Mask');
[x,y]=getline('closed');
m=poly2mask(x,y,size(I,1),size(I,2));
hold on; imshow(m); hold off;


seg = simpleseg(I, m, 1000, 35, 170, 0.02); %-- Run segmentation
subplot(2,2,1); imshow(seg); title('Final Mask of phi<=0');