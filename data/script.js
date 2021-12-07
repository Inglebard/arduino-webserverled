/*ajax functions to match web server*/
function sendLedData(idLed)
{
  var ledObj=ledObjs[idLed];
  $.ajax({
    url: "/led/?id="+idLed+"&r="+ledObj.r+"&g="+ledObj.g+"&b="+ledObj.b
  });
  
}
function sendAllLedData()
{
  var rgbValue=hexToRgb($("#all_led_color_selector").val());
  var brightness = $("#all_led_brightness_selector").val();  
  
  $.ajax({
    url: "/allleds/?r="+rgbValue.r+"&g="+rgbValue.g+"&b="+rgbValue.b
  });
  
}
function sendParameterData(name,value)
{
  
  $.ajax({
    url: "/parameters/?name="+name+"&value="+value
  });
  
}
function sendModeData(mode)
{
  $.ajax({
    url: "/modes/?mode="+mode
  });  
}
function sendTemplateData(template)
{
  $.ajax({
    url: "/templates/?template="+template,
	dataType: 'json',
	success: function (ledObjs) {
	  //console.log(ledObjs);
	  for(ledObj_index in ledObjs )
	  {
		ledObj=ledObjs[ledObj_index];
		$("#color_selector_"+ledObj_index).val("#"+rgb2hex(ledObj.r,ledObj.g,ledObj.b));
   		$(".color_viewer_"+ledObj_index).css('backgroundColor', "#"+rgb2hex(ledObj.r,ledObj.g,ledObj.b));
	  }
	}
  });  
}

sendTemplateData
/*end ajax functions to match web server*/

//build leds listing and event
function refreshUi()
{
  var templateLedObj = `
  <div class="ib ledObj">
    <input type="hidden" class="ledObj_num" value="%LED_ID%">
    <div class="ledObj_inner">	
      <span class="ib name">LED %LED_ID%</span>
      <div>					
        <div class="ib color">	
          <div style="background-color:%COLOR_VALUE%" class="color_viewer color_viewer_%LED_ID%">
          </div>						
          <input data-ledid="%LED_ID%" type="color" class="color_selector color_selector_%LED_ID%" id="color_selector_%LED_ID%" value="%COLOR_VALUE%">
        </div>
      </div>			
    </div>   
  </div> 
  `;
  $("#listing_leds").html("");
  for(ledObj_index in ledObjs )
  {
    ledObj=ledObjs[ledObj_index];
    output = templateLedObj.replaceAll('%LED_ID%', ledObj_index);
    output = output.replaceAll('%COLOR_VALUE%', "#"+rgb2hex(ledObj.r,ledObj.g,ledObj.b));
    $("#listing_leds").append(output);
  }    
  
  
  $(".listing_leds .color_selector").change(function() {
    var color_viewer = $(this).siblings(".color_viewer");
    color_viewer.css('backgroundColor', $(this).val());
    var ledId= $(this).data("ledid");
    var rgbValue=hexToRgb($(this).val());
    ledObjs[ledId].r = rgbValue.r;
    ledObjs[ledId].g = rgbValue.g;
    ledObjs[ledId].b = rgbValue.b;
    sendLedData(ledId);
  });

  $(".listing_leds .color_viewer").click(function() {
    var color_selector = $(this).siblings(".color_selector");
    color_selector.trigger("click");
  });
  
}

/* colors functions */
function rgb2hex(r,g,b) {
  var hexValue="";
  hexValue = ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
  return hexValue;
}


function hexToRgb(hex){
    var c;
    var rgbObj={};
    if(/^#([A-Fa-f0-9]{3}){1,2}$/.test(hex)){
        c= hex.substring(1).split('');
        if(c.length== 3){
            c= [c[0], c[0], c[1], c[1], c[2], c[2]];
        }
        c= '0x'+c.join('');
        rgbObj.r = (c>>16)&255;
        rgbObj.g = (c>>8)&255;
        rgbObj.b = (c)&255;
        return rgbObj;
        
    }
    throw new Error('Bad Hex');
}

/* end colors functions */

$(document).ready(function() {
  // individual led ui and event 
  refreshUi();

  /* Template Selector */ 
  $(".template_selector").click(function(e) {
    e.preventDefault();
    var template_value = $(this).data("template");
    sendTemplateData(template_value);
  });
  
  /* All leds ui and events */
  $(".listing_custom_control .color_selector").change(function() {
    $(".color_viewer").css('backgroundColor', $(this).val());
    sendAllLedData();
  });

  $(".listing_custom_control .color_viewer").click(function() {
    var color_selector = $(this).siblings(".color_selector");
    color_selector.trigger("click");
  });
  
  $(".listing_custom_control .color_viewer").click(function() {
    var color_selector = $(this).siblings(".color_selector");
    color_selector.trigger("click");
  });
  /* end All leds ui and events */
  
  // init slider value
  $("#brightness_selector").val(init_brightness_value);
  $("#direction_selector").val(init_direction_value);
  $("#delay_selector").val(init_delay_value);
  
  /* modes functions */
  $(".mode_selector").each(function(){
    if($(this).data("real_value") == init_mode_value)
    {
      $(this).addClass("active");
    }    
  });
  $(".mode_selector").click(function() {
    var mode_real_value = $(this).data("real_value");
    $(".mode_selector").removeClass("active");
    $(this).addClass("active");
    sendModeData(mode_real_value);
  });
  
  /* end modes functions */
  
  /* parameters functions */
  $("#brightness_selector").change(function() {
    var brightness_value = $(this).val();
    sendParameterData("brightness",brightness_value)
  });
  $("#direction_selector").change(function() {
    var direction_value = $(this).val();
    sendParameterData("direction",direction_value)
  });

  $("#delay_selector").change(function() {
    var delay_value = $(this).val();
    sendParameterData("delay",delay_value)
  });
  /* end parameters functions */
  
});
