public fun avaspec__add(in _path, out _nidout)
{
  DevAddStart(_path,'avaspec',19,_nidout);
  DevAddNode(_path//':COMMENT','TEXT',*,*,_nid);
  DevAddNode(_path//':MODEL','TEXT',"Dragon Fly",*,_nid);
  DevAddNode(_path//':MAX_FRAMES', 'NUMERIC', 60, '/noshot_write', _nid);
  DevAddNode(_path//':SPECTROMETER_NO', 'NUMERIC', 0, '/noshot_write', _nid);
  DevAddNode(_path//':INTEGRATION', 'NUMERIC', 0.200, '/noshot_write', _nid);
  DevAddNode(_path//':AVERAGE', 'NUMERIC', 1, '/noshot_write', _nid);
  DevAddNode(_path//':MAX_SPECTRA', 'NUMERIC', 2000, '/noshot_write', _nid);
  DevAddNode(_path//':TRIGGERS', 'NUMERIC', 0:10:1, '/noshot_write', _nid);
  DevAddNode(_path//':EXT_TRIG_MODE', 'NUMERIC', 1  , '/noshot_write', _nid);  /* TRIG_MODE_0 */
  DevAddNode(_path//':ISO_SPEED', 'NUMERIC', 100, '/noshot_write', _nid); /* 100, 200, 400 */
  DevAddNode(_path//':FRAME_RATE', 'NUMERIC', 30., '/noshot_write', _nid); /* 1.875, 3.75, 7.5, 15, 30, 60 */
  DevAddNode(_path//':TRIG_ON   ', 'NUMERIC', 0 , '/noshot_write', _nid); /* 0 - no trigger , 1 - trigger expected */
  DevAddNode(_path//':FRAMES','SIGNAL',*,'/write_once/compress_on_put/nomodel_write',_nid);
  DevAddNode(_path//':REQUESTED', 'NUMERIC', 0 : 59 : 1, '/noshot_write', _nid);
  DevAddNode(_path//':CAMERA', 'TEXT', *, '/noshot_write', _nid);
  DevAddNode(_path//':TRIGGER', 'NUMERIC', 0.0, *, _nid); 
  DevAddAction(_path//':INIT_ACTION','INIT','INIT',50,'CAMAC_SERVER',_path,_nid);
  DevAddAction(_path//':STORE_ACTION','STORE','STORE',50,'CAMAC_SERVER',_path,_nid);
  DevAddEnd();
  return(1);
}
