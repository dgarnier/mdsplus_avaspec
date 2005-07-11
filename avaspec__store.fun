public fun dc1394__store(as_is _nid, optional _method)
{
   _AVASPEC_HEAD = 0;
   _AVASPEC_COMMENT = 1;
   _AVASPEC_SPECT_NO=2;
   _AVASPEC_INT_TIME=3;
   _AVASPEC_AVERAGE=4;
   _AVASPEC_MAX_SPECTRA=5;
   _AVASPEC_DYNAMIC_DARK=6;
   _AVASPEC_TRIGGERS=7;
   _AVASPEC_CHANNEL_1=8
   _AVASPEC_CHANNEL_1_DARK=9;
   _AVASPEC_INIT_ACTION = 10;
   _AVASPEC_STORE_ACTION = 11;

  _max_spectra = if_error(DevNodeRef(_nid, _AVASPEC_MAX_SPECTRA), 1);
  _spec_no = if_error(DevNodeRef(_nid, _AVASPEC_SPEC_NO), 0);

  avaspec->Stop(val(_spec_no));

  _num_spectra = avaspec->NumSpectra(val(_spec_no));
  
  if (_num_spectra <= 0) {
     write(*, "No spectra aken");
     return(1);
  }
  
  _num_waves = avaspec->NumWavelengths(val(_spec_no));
  
  _waves = zero(_num_waves, 0.0E0);
  _dark = zero(_num_waves, 0w);
  _spectra = zero([_num_waves,_num_spectra], 0w);

  avaspec->ReadDark((val(_spec_no)), 0, ref(_dark));
  avaspec->ReadWavelengths((val(_spec_no)), 0, ref(_waves));
  avaspec->ReadSpectra((val(_spec_no)), 0, ref(_spectra));

  avaspec->Destroy((val(_spec_no)));
  
  _triggers =  DevNodeRef(_nid, _AVASPEC_TRIGGERS);
  _int_time = DevNodeRef(_nid, _AVASPEC_INT_TIME);
  
  _taxis = MAKE_WITH_UNITS((_int_time+_triggers)
  _wlaxis = MAKE_WITH_UNITS((_waves),"Angstrom");
  
  
  _signal = make_signal(MAKE_WITH_UNITS((_spectra), "Counts"), *, make_range( 0, _width, 1), make_range( 0, _height, 1), MAKE_DIM(MAKE_WINDOW(0, _num_frames-1,  _trigger), _times));
    
  _status = TreeShr->TreePutRecord(val(DevHead(_nid) + _DC1394_SPECTRA),xd(_signal),val(0));
  

  return(_status);
}
