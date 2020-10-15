	/*
	** Reads a greyscale image from the comand line, and creates
	** a segmentation of regions based upon similar greyscale areas.
	** This version demonstrates how to use a paint-fill technique
	** along with region criteria to do segmentation.
	**
	** Good demonstration on targets.ppm.
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Windows.h>
#define SQR(x) ((x)*(x))

void reg_grow(const unsigned char * image, HDC hDC,int r,int c,int R,int G, int B,int ROWS,int COLS,int threshold,int max_dist)

{
	unsigned char	*labels;
	char		header[80];
	int		BYTES, r2, c2;
	int		*indices, i;
	int		RegionSize, *RegionPixels, TotalRegions;
	double		avg, var;
	void		RegionGrow();


	/* Allocate memory for images.  Read image (raw grey). */

	/* segmentation image = labels; calloc initializes all labels to 0 */
	labels = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
	/* used to quickly erase small grown regions */
	indices = (int *)calloc(ROWS*COLS, sizeof(int));

	TotalRegions = 0;

	if (labels[r*COLS + c] != 0)
		return;
			avg = var = 0.0;	/* compute average and variance in 7x7 window */
			for (r2 = -3; r2 <= 3; r2++)
				for (c2 = -3; c2 <= 3; c2++)
					avg += (double)(image[(r + r2)*COLS + (c + c2)]);
			avg /= 49.0;
			for (r2 = -3; r2 <= 3; r2++)
				for (c2 = -3; c2 <= 3; c2++)
					var += SQR(avg - (double)image[(r + r2)*COLS + (c + c2)]);
			var = sqrt(var) / 49.0;
			if (var < 1.0)	/* condition for seeding a new region is low var */
			{
				// printf("%d,%d avg=%lf var=%lf\n",r,c,avg,var);
				TotalRegions++;
				if (TotalRegions == 255)
				{
					printf("Segmentation incomplete.  Ran out of labels.\n");
					return;
				}
				RegionGrow(image, labels, ROWS, COLS, r, c, 0, TotalRegions,
					indices, &RegionSize,threshold,max_dist);
				if (RegionSize < 100)
				{	/* erase region (relabel pixels back to 0) */
					for (i = 0; i < RegionSize; i++)
						labels[indices[i]] = 0;
					TotalRegions--;
				}
				else
					printf("Region labeled %d is %d in size\n", TotalRegions, RegionSize);
			}
		
		for (r2 = 0; r2 < ROWS; r2++) {
			for (c2 = 0; c2 < COLS; c2++) {
					i = r2 * COLS + c2;
					if (labels[i] != 0) {
						//if play mode
						SetPixel(hDC, c2,r2, RGB(R,G,B));	/* color the animation pixel blue */
						Sleep(1); //pause for 1 ms
					}
				}
			}
}


	/*
	** Given an image, a starting point, and a label, this routine
	** paint-fills (8-connected) the area with the given new label
	** according to the given criteria (pixels close to the average
	** intensity of the growing region are allowed to join).
	*/

#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */

void RegionGrow(const unsigned char *image,	/* image data */
		unsigned char *labels,	/* segmentation labels */
		int ROWS,int COLS,	/* size of image */
		int r,int c,		/* pixel to paint from */
		int paint_over_label,	/* image label to paint over */
		int new_label,		/* image label for painting */
		int *indices,		/* output:  indices of pixels painted */
		int *count,
		int threshold,
		int cent_dist)		/* output:  count of pixels painted */
{
int	r2,c2;
int	queue[MAX_QUEUE],qh,qt;
int	average,total;	/* average and total intensity in growing region */

*count=0;
if (labels[r*COLS+c] != paint_over_label)
  return;
labels[r*COLS+c]=new_label;
average=total=(int)image[r*COLS+c];
if (indices != NULL)
  indices[0]=r*COLS+c;
queue[0]=r*COLS+c;
qh=1;	/* queue head */
qt=0;	/* queue tail */
(*count)=1;
while (qt != qh)
  {
  if ((*count)%50 == 0)	/* recalculate average after each 50 pixels join */
    {
    average=total/(*count);
    // printf("new avg=%d\n",average);
    }
  for (r2=-1; r2<=1; r2++)
    for (c2=-1; c2<=1; c2++)
      {
      if (r2 == 0  &&  c2 == 0)
        continue;
      if ((queue[qt]/COLS+r2) < 0  ||  (queue[qt]/COLS+r2) >= ROWS  ||
	  (queue[qt]%COLS+c2) < 0  ||  (queue[qt]%COLS+c2) >= COLS)
        continue;
      if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label)
        continue;
		/* test criteria to join region */
      if (abs((int)(image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2])
		-average) > threshold)
        continue;
      labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;
      if (indices != NULL)
        indices[*count]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
      total+=image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
      (*count)++;
      queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
      qh=(qh+1)%MAX_QUEUE;
      if (qh == qt)
        {
        printf("Max queue size exceeded\n");
        exit(0);
        }
      }
  qt=(qt+1)%MAX_QUEUE;
  }
}

