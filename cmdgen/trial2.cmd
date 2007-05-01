&program : &cmds Quit *
	  ;
%INTERFACE <indxr>
&cmds :
	  : &cmds &cmd
	  ;
&cmd  : Lamp A On * { data(A on); }
	  : Lamp B On * { data(B on); }
	  ;
