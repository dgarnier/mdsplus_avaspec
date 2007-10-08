public fun avaspec__add(in _path, out _nidout)
{
  DevAddStart(_path,'avaspec',13,_nidout);
  DevAddNode(_path//':COMMENT','TEXT',*,*,_nid);
  DevAddNode(_path//':SPECTROMETER_NO', 'NUMERIC', 0, '/noshot_write', _nid);
  DevAddNode(_path//':INT_TIME', 'NUMERIC', 0.200, '/noshot_write', _nid);
  DevAddNode(_path//':AVERAGE', 'NUMERIC', 1, '/noshot_write', _nid);
  DevAddNode(_path//':MAX_SPECTRA', 'NUMERIC', 2000, '/noshot_write', _nid);
  DevAddNode(_path//':DYNAMIC_DARK', 'NUMERIC', 1, '/noshot_write', _nid);
  DevAddNode(_path//':TRIG_EVENT',   'TEXT', *, '/noshot_write', _nid);
  DevAddNode(_path//':TRIGGERS', 'NUMERIC', 0:10:1, '/noshot_write', _nid);
  
  DevAddNode(_path//':CHANNEL_1','SIGNAL',*,'/write_once/compress_on_put/nomodel_write',_nid);
  DevAddNode(_path//':CHANNEL_1:DARK','SIGNAL',*,'/write_once/compress_on_put/nomodel_write',_nid);
  DevAddAction(_path//':INIT_ACTION','INIT','INIT',50,'AVASPEC_SERVER',_path,_nid);
  DevAddAction(_path//':TRIGGER_ACTION','PULSE_ON','PULSE',50,'AVASPEC_SERVER',_path,_nid);
  DevAddAction(_path//':STORE_ACTION','STORE','STORE',50,'AVASPEC_SERVER',_path,_nid);
  DevAddEnd();
  return(1);
}
