void
GetFormat7Capabilities(raw1394handle_t handle, nodeid_t node, Format7Info *info)
{
  int i, f, err;
  quadlet_t value;
  
  err=dc1394_query_supported_modes(handle, node, FORMAT_SCALABLE_IMAGE_SIZE, &value);
  if (!err) MainError("Could not query Format7 supported modes");
  for (i=0,f=MODE_FORMAT7_MIN;f<MODE_FORMAT7_MAX;f++,i++)
    {
      info->mode[i].present= (value & (0x1<<(31-i)) );
      if (info->mode[i].present) // check for mode presence before query
	{
	  err=1;
	  err*=dc1394_query_format7_max_image_size(handle,node,f,&info->mode[i].max_size_y,&info->mode[i].max_size_x);
	  err*=dc1394_query_format7_unit_size(handle,node,f,&info->mode[i].step_y,&info->mode[i].step_x);
	  err*=dc1394_query_format7_image_position(handle,node,f,&info->mode[i].pos_y,&info->mode[i].pos_x);
	  err*=dc1394_query_format7_image_size(handle,node,f,&info->mode[i].size_y,&info->mode[i].size_x);
	  
	  err*=dc1394_query_format7_pixel_number(handle,node,f,&info->mode[i].pixnum);
	  err*=dc1394_query_format7_byte_per_packet(handle,node,f,&info->mode[i].bpp);
	  err*=dc1394_query_format7_packet_para(handle,node,f,&info->mode[i].min_bpp,&info->mode[i].max_bpp);
	  err*=dc1394_query_format7_total_bytes(handle,node,f,&info->mode[i].total_bytes);
	  
	  // TODO: get color coding id
	  err*=dc1394_query_format7_color_coding(handle,node,f,&info->mode[i].color_coding);
	  if (!err) MainError("Got a problem querying format7 capabilities");
	  
	}
    }
  info->edit_mode = MODE_FORMAT7_MIN;

}
