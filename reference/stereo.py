from PIL import Image
from numpy import *
from pylab import *
#from scipy import *
import scipy.ndimage.filters as flt
# import numpy as np
import matplotlib.pyplot as plt
import scipy.misc

def plane_sweep_ncc(im_l,im_r,start,steps,wid):
	m,n = im_l.shape
	mean_l = zeros((m,n))
	mean_r = zeros((m,n))
	s = zeros((m,n))
	s_l = zeros((m,n))
	s_r = zeros((m,n))
	dmaps = zeros((m,n,steps))
	sums = zeros((m,n,steps))
	
	# flt.uniform_filter1d(im_l,wid,1,mean_l)
	# flt.uniform_filter1d(im_r,wid,1,mean_r)
	
	# plt.gray()
	# plt.imshow(mean_l)
	# plt.show()
	# scipy.misc.imsave('debug1L.png',im_l)
	scipy.misc.imsave('debug1R.png',im_r)
	# scipy.misc.imsave('debug2L.png',mean_l)
	scipy.misc.imsave('debug2R.png',mean_r)
	#return mean_l
	
	norm_l = flt.sobel(im_l)# - mean_l
	norm_r = flt.sobel(im_r)# - mean_r
	
	# scipy.misc.imsave('debug3L.png',norm_l)
	scipy.misc.imsave('debug3R.png',norm_r)
	
	#flt.uniform_filter1d(norm_r*norm_r,wid,1,s_r)
	
	for displ in range(steps):
		print displ
		rolled_l = roll(norm_l,-displ-start)
		# rolled_l = roll(norm_l,-(displ*wid)-start)
		
		#flt.uniform_filter1d(rolled_l*rolled_l,wid,1,s_l)
		#flt.uniform_filter1d(rolled_l*norm_r,wid,1,s)
		
		flt.uniform_filter1d(abs(rolled_l*norm_r),wid,1,s)
		
		#scipy.misc.imsave('debug4s_'+repr(displ)+'.png',s)
		#scipy.misc.imsave('debug4sL_'+repr(displ)+'.png',s_l)
		dmaps[:,:,displ] = s #s/sqrt(s_l*s_r)
		sums[:,:,displ] = s #abs(rolled_l*norm_r) #rolled_l+norm_r #(s_l+s_r) #s
		scipy.misc.imsave('debug5dmap'+repr(displ)+'.png', sums[:,:,displ])   #flt.sobel(dmaps[:,:,displ]))
	scipy.misc.imsave('debug4sL.png',s_l)
	return argmax(dmaps,axis=2)
	
def plane_sweep_gauss(im_l,im_r,start,steps,wid):
	m,n = im_l.shape
	mean_l = zeros((m,n))
	mean_r = zeros((m,n))
	s = zeros((m,n))
	s_l = zeros((m,n))
	s_r = zeros((m,n))
	dmaps = zeros((m,n,steps))
	
	flt.gaussian_filter1d(im_l,wid,1,0,mean_l)
	flt.gaussian_filter1d(im_r,wid,1,0,mean_r)
	
	norm_l = im_l - mean_l
	norm_r = im_r - mean_r
	
	for displ in range(steps):
		print displ
		
		rolled_l = roll(norm_l,-displ-start)
		flt.gaussian_filter1d(rolled_l*norm_r,wid,1,0,s)
		flt.gaussian_filter1d(rolled_l*rolled_l,wid,1,0,s_l)
		flt.gaussian_filter1d(norm_r*norm_r,wid,1,0,s_r)
		
		dmaps[:,:,displ] = s/sqrt(s_l*s_r)
	return argmax(dmaps,axis=2)
	
	
	
	
	
	
def compute_harris_response(im,sigma=3):
	imx = zeros(im.shape)
	imy = zeros(im.shape)
	flt.gaussian_filter(im, (sigma,sigma), (0,1), imx)
	flt.gaussian_filter(im, (sigma,sigma), (1,0), imy)
	
	Wxx = flt.gaussian_filter(imx*imx, sigma)
	Wxy = flt.gaussian_filter(imx*imy, sigma)
	Wyy = flt.gaussian_filter(imy*imy, sigma)
	
	Wdet = Wxx*Wyy - Wxy**2
	Wtr = Wxx + Wyy
	
	return Wdet / Wtr
	
def get_harris_points(harrisim, min_dist=10, threshold=0.1):
	corner_threshold = harrisim.max() * threshold
	harrisim_t = (harrisim > corner_threshold) * 1
	
	coords = array(harrisim_t.nonzero()).T
	candidate_values = [harrisim[c[0],c[1]] for c in coords]
	
	index = argsort(candidate_values)
	
	allowed_locations = zeros(harrisim.shape)
	allowed_locations[min_dist:-min_dist,min_dist:-min_dist] = 1
	
	filtered_coords = []
	for i in index:
		if allowed_locations[coords[i,0], coords[i,1]] == 1:
			filtered_coords.append(coords[i])
			allowed_locations[(coords[i,0]-min_dist):(coords[i,0]+min_dist),(coords[i,1]-min_dist):(coords[i,1]+min_dist)] = 0
			
	return filtered_coords
	
	
	
def plot_harris_points(image,filtered_coords):
	plt.figure()
	plt.gray()
	plt.imshow(image)
	plt.plot([p[1] for p in filtered_coords],[p[0] for p in filtered_coords],'.')
	plt.axis('off')
	plt.show()