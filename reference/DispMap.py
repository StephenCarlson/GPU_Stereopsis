from PIL import Image
from numpy import *
from pylab import *
#import numpy as np
#import matplotlib.pyplot as plt
import scipy.ndimage.filters as flt
import scipy.misc
import stereo

print 'Program Start'

# imageNameLeft  = 'CombatLeft'
# imageNameRight = 'CombatRight'
# imageNameLeft  = 'BinocularL'
# imageNameRight = 'BinocularR'
# imageNameLeft = 'simple2L'
# imageNameRight = 'simple2R'
imageNameLeft = 'im2'
imageNameRight = 'im6'

im_l = array(Image.open(imageNameLeft+'.png' ).convert('L'),'f')
im_r = array(Image.open(imageNameRight+'.png').convert('L'),'f')

# steps = 80
# start = 0
# wid = {15,25}

steps = 20
start = 0
wid = {3,5,7}

print 'Start Sequence'

# harrisim = stereo.compute_harris_response(im_l)
# filtered_coords = stereo.get_harris_points(harrisim,6)
# stereo.plot_harris_points(im_l,filtered_coords)


for i in wid:
	res = stereo.plane_sweep_ncc(im_l,im_r,start,steps,i)
	modifier = 'AR'
	
	# res = stereo.plane_sweep_gauss(im_l,im_r,start,steps,i)
	# modifier = 'G'
	
	# res = stereo.plane_sweep_ncc(im_l,im_r,start,steps,i)
	# modifier = 'R' # Remove the High-Pass Prefilter, process the raw image
	
	# res = stereo.plane_sweep_gauss(im_l,im_r,start,steps,i)
	# modifier = 'RG' # Remove the High-Pass Prefilter, process the raw image
	
	# res = stereo.plane_sweep_ncc(im_l,im_r,start,steps,i)
	# modifier = 'S'
	# flt.uniform_filter(res,7,res)
	
	scipy.misc.imsave(imageNameLeft+'_'+modifier+'_'+repr(steps)+'_'+repr(start)+'_'+repr(i)+'.png',res)





# Flight Sim
# steps = 10
# start = 0
# wid = 5

# Cones
# steps = 60
# start = 15
# wid = 9





# im_l = array(Image.open('CombatLeft.png' ).convert('L'),'f')
# im_r = array(Image.open('CombatRight.png').convert('L'),'f')
# im_l = array(Image.open('im2.png').convert('L'),'f')
# im_r = array(Image.open('im6.png').convert('L'),'f')
# im_l = array(Image.open('urban_0nm6_left.png').convert('L'),'f')
# im_r = array(Image.open('urban_0nm6_right.png').convert('L'),'f')
# im_l = array(Image.open('sky_3nm_left.png').convert('L'),'f')
# im_r = array(Image.open('sky_3nm_right.png').convert('L'),'f')



