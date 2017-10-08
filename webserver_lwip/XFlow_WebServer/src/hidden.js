var g_should_update = false;

var g_setup_on = false;
var g_flow_waring = 0;
var g_flow_fault = 0;
var g_leak_rsp = 0;
var g_stb_delay = 0;
var g_startup_leak = 0;

function copy_innerText(src,dest)
{
	var iT;

	iT = document.getElementById(src).innerText;
	eleDest = window.parent.status_frame.document.getElementById(dest);
	if (eleDest.src != iT)
	{
		eleDest.src = iT;
	}
}

function copy_setup_value(id,val) {
		window.parent.info_frame.document.getElementById(id).value = val;
}

function set_setup_selection(id,sel) {
	var opts = window.parent.info_frame.document.getElementById(id).getElementsByTagName("option");
	for (i=0; i<opts.length; i++)
	{
		if(opts[i].value==sel)
			opts[i].selected=true;
		else
			opts[i].selected=false;
	}
}

function update_status()
{
	copy_innerText("flow0_src","id_flow0_img");
	copy_innerText("flow1_src","id_flow1_img");
	copy_innerText("flow2_src","id_flow2_img");
	copy_innerText("status0_src","id_status0_img");
	copy_innerText("status1_src","id_status1_img");
	copy_innerText("status2_src","id_status2_img");
	
	if (window.parent.status_frame.document.flow_unit_img.src != document.embedded_data.flow_unit.value)
	{
		window.parent.status_frame.document.flow_unit_img.src = document.embedded_data.flow_unit.value;
	}
	
	if(g_setup_on && g_should_update) {
		copy_setup_value("id_setup_flow_warning",g_flow_waring);
		copy_setup_value("id_setup_flow_fault",g_flow_fault);
		set_setup_selection("id_setup_lr",g_leak_rsp);
		set_setup_selection("id_setup_sd",g_stb_delay);
		set_setup_selection("id_setup_sl",g_startup_leak);
	}
	
}

function update_info()
{
	var lobj;
	lobj = window.parent.info_frame.document.info_img;

	if (lobj != null) {
		lobj = document.getElementById("info_src").innerText;
		if (window.parent.info_frame.document.info_img.src != lobj)
		{
			window.parent.info_frame.document.info_img.src = lobj;
		}
	}
}