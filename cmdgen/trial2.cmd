&program : &cmds
	  ;
&cmds :
	  : &cmds &cmd
	  ;
&cmd  : Lamp A On * { data(A on); }
	  : Lamp B On * { data(B on); }
	  ;
