public fun avaspec__PART_NAME(as_is _nid, optional in _method)
{
  _name = ([
  '',
':COMMENT',
':SPECTROMETER_NO',
':INT_TIME',
':AVERAGE', 
':MAX_SPECTRA',
':DYNAMIC_DARK',
':TRIGGERS', 
':CHANNEL_1',
':CHANNEL_1:DARK',
':INIT_ACTION',
':STORE_ACTION'])[getnci(_nid,'conglomerate_elt')-1];
  return(trim(_name));
}
