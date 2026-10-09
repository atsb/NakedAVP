#include "txioctrl.h"

#ifdef MaxImageGroups

#include "list_tem.hpp"

struct ImageNumber
{
	int group;
	int offset;

	ImageNumber(){}
	ImageNumber(int g,int o) : group(g), offset(o) {}

	inline int operator == (ImageNumber const & in2) const
		{ return group==in2.group && offset==in2.offset; }
	inline int operator != (ImageNumber const & in2) const
		{ return ! operator == (in2); }
};

static List<ImageNumber> imgs_used_by_group[MaxImageGroups];

static List<int> * grps_used_by_image[MaxImageGroups][MaxImages];

void MarkImageInUseByGroup(int img_group, int img_num_offset, int group_using)
{
	if (imgs_used_by_group[group_using].contains(ImageNumber(img_group,img_num_offset))) return;
	
	imgs_used_by_group[group_using].add_entry(ImageNumber(img_group,img_num_offset));

	List<int> * & grps_used_by_imageR = grps_used_by_image[img_group][img_num_offset];

	if (!grps_used_by_imageR)
		grps_used_by_imageR = new List<int>(img_group); // includes itself

	if (!grps_used_by_imageR->contains(group_using))
		grps_used_by_imageR->add_entry(group_using);
}

int IsImageInUse(int img_group, int img_num_offset)
{
	if (grps_used_by_image[img_group][img_num_offset])
		return 1;
	else
		return 0;
}

int CanDeleteImage(int img_group, int img_num_offset)
{
	if (grps_used_by_image[img_group][img_num_offset])
	{
		if (grps_used_by_image[img_group][img_num_offset]->contains(img_group))
		{
			grps_used_by_image[img_group][img_num_offset]->delete_entry(img_group); // remove itself
			if (!grps_used_by_image[img_group][img_num_offset]->size())
			{
				delete grps_used_by_image[img_group][img_num_offset];
				grps_used_by_image[img_group][img_num_offset] = 0;
				return 1;
			}
			else
				return 0;
		}
		else
			return 0;
	}
	else
		return 1;
}

void ImageGroupFreed(int img_group)
{
	while (imgs_used_by_group[img_group].size())
	{
		ImageNumber used = imgs_used_by_group[img_group].first_entry();

		List<int> * & grps_used_by_imageR = grps_used_by_image[used.group][used.offset];

		grps_used_by_imageR->delete_entry(img_group);

		if (!grps_used_by_imageR->size())
		{
			NowDeleteImage(used.group,used.offset);
			delete grps_used_by_imageR;
			grps_used_by_imageR = 0;
		}
		
		imgs_used_by_group[img_group].delete_first_entry();
	}
}

void EnumSharedImages(int group_num, int numimages, ImageNumberCallbackFunction callback_fn, void * user)
{
	for (int i=0; i<numimages; ++i)
	{
		if (grps_used_by_image[group_num][i])
			callback_fn(i,user);
	}
}

void EnumLeftoverImages(int group_num, int numimages, ImageNumberCallbackFunction callback_fn, void * user)
{
	for (int i=numimages; i<MaxImages; ++i)
	{
		if (grps_used_by_image[group_num][i])
			callback_fn(i,user);
	}
}

#endif // MaxImageGroups
