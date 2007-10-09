public fun avaspec__init(as_is _nid, optional in _method)
{
   _AVASPEC_HEAD = 0;
   _AVASPEC_COMMENT = 1;
   _AVASPEC_SPECT_NO=2;
   _AVASPEC_INT_TIME=3;
   _AVASPEC_AVERAGE=4;
   _AVASPEC_MAX_SPECTRA=5;
   _AVASPEC_DYNAMIC_DARK=6;
   _AVASPEC_TRIG_EVENT=7;
   _AVASPEC_TRIGGERS=8;
   _AVASPEC_CHANNEL_1=9;
   _AVASPEC_CHANNEL_1_DARK=10;
   _AVASPEC_INIT_ACTION = 11;
   _AVASPEC_TRIGGER_ACTION=12;
   _AVASPEC_STORE_ACTION = 13;

  _spec_no = if_error(DevNodeRef(_nid, _AVASPEC_SPEC_NO), 0);
  _int_time = float(if_error(DevNodeRef(_nid, _AVASPEC_INT_TIME), 0.25));
  _trig_event = string(if_error(DevNodeRef(_nid, _AVASPEC_TRIG_EVENT), "");
  _triggers = if_error(DevNodeRef(_nid, _AVASPEC_TRIGGERS), 0:1:10);
  _average = if_error(DevNodeRef(_nid, _AVASPEC_AVERAGE), 1);
  _dynamic = if_error(DevNodeRef(_nid, _AVASPEC_DYNAMIC_DARK), 1);
  _max_spectra = if_error(DevNodeRef(_nid, _AVASPEC_MAX_SPECTRA), 1);

  _status = avaspec->Init(val(_spec_no), val(_int_time), val(_average), val(_dynamic), val(_max_spectra));
   return(_status != -1);
} 
