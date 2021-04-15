<?php
/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file index.php
 * This is a a tool script to get an overview of settings in api specification file.
 *
 * Description:
 * Loads a specification file and do different outputs of the contents.
 * ** shows all Calls with there arguments
 * ** shows calls with there analyses
 * ** shows Calls with the possibility to notice some TODOs ^^
 * ** overview over all todos and notices
 *
 *
 *  @date 07.04.2011
 *  @author Mathias Korepkat
 */

//output 
// css to desgin the page
//TODO valide html with doctype defenition
echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"
                      \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"de\" lang=\"de\"><html><head><title>Specification parsing tool</title><style type=\"text/css\">
table {border:none;}
td,th {border: 1px solid #0000AA;
         vertical-align:top;
      }
td.emptyRow {border: none;
      }
td.l1,th.l1 {background-color:#DDDDFF;}
td.head,th.head {background-color:#A0A0D0;}
a:link,a:active,a:visited  {color:#440000;
                            text-decoration:none;
                           } 
a:hover {color:#FF0000;}

div.marked {background-color:#FFFF00;}
</style><head><body>";
echo "<h1>Mpi Specification - Parsing Tool</h1><br/><br/>";

//=================
//= Variablen
//=================

$CALLS = array();
//initialize
$CALLS = loadCalls("mpi_specification.xml");


//=================
//= Navigation
//=================

// view tables with url:
// index.php?view=1 -> function
// index.php?view=2 -> analyses
$url = "";
if(isset($_GET["sat"]))$url .= "&sat=1";
if(isset($_GET["sag"]))$url .= "&sag=1";
if(isset($_GET["sai"]))$url .= "&sai=1";
if(isset($_GET["sao"]))$url .= "&sao=1";
if(isset($_GET["sano"]))$url .= "&sano=1";
if(isset($_GET["hl"]))$url .= "&hl=1";
if(isset($_GET["fin"]) && trim($_GET["fin"])!= "")$url .= "&fin=".$_GET["fin"];
if(isset($_GET["ft"]) && trim($_GET["ft"])!= "")$url .= "&ft=".$_GET["ft"];
if(isset($_GET["fn"]) && trim($_GET["fn"])!= "")$url .= "&fn=".$_GET["fn"];
if(isset($_GET["fc"]) && trim($_GET["fc"])!= "")$url .= "&fc=".$_GET["fc"];
if(isset($_GET["fan"]) && trim($_GET["fan"])!= "")$url .= "&fan=".$_GET["fan"];
if(isset($_GET["fac"]) && trim($_GET["fac"])!= "")$url .= "&fac=".$_GET["fac"];


echo "<b>menu:</b><ul>
    <li><a href=\"index.php?view=s$url\" title=\"for bugfixes\">struktur</a></li>
    <li><a href=\"index.php?view=c$url\" title=\"for bugfixes\">coustomize view</a></li>
    <li><a href=\"index.php?view=1$url\" title=\"functions\">overview over MPI Calls and there arguments</a></li>
    <li><a href=\"index.php?view=2$url\" title=\"analyses\">overview over MPI Calls and there analyses</a></li>
    <li><a href=\"index.php?view=3$url\" title=\"operation\">overview over MPI Calls and there operations</a></li>
    <li><a href=\"index.php?view=4$url\" title=\"analyses mapped\">overview over MPI Calls and there analyses mapped to function arguments</a></li>
    <li><a href=\"index.php?view=5$url\" title=\"operations mapped\">overview over MPI Calls and there operation mapped to function arguments</a></li>
    <li><a href=\"index.php?view=6$url\" title=\"Todos\">overview over MPI Calls and TODOs</a></li>
    <li><a href=\"index.php?view=8$url\" title=\"analyses mapped and TODOs\">overview over MPI Calls and there analyses mapped to function arguments and todos</a></li>
    <li><a href=\"index.php?view=7$url\" title=\"Todo\">overview over TODOs</a></li>
</ul>";
isset($_GET["view"])?$val=$_GET["view"]:$val=1;
switch($val)
{
    case "s":printvarDump($CALLS);
    break;
    case "c":printCoustomizeView($CALLS);
    break;
    case "1":printFunctions($CALLS);
    break;
    case "2":printAnalyses($CALLS);
    break;
    case "3":printOperation($CALLS);
    break;
    case "4":printAnalysesMapped($CALLS);
    break;
    case "5":printOperationMapped($CALLS);
    break;
    case "6":printFunctionsTodo($CALLS);
    break;
    case "8":printAnaTodos($CALLS);
    break;
    case "7":printTodos($CALLS);
    break;
    default:printFunctions($CALLS);
    break;
    
}
//=================
//= Functionen
//=================


// LOADS Information from xml file
function loadCalls($file)
{
    $CALLS = array();
    
    if (file_exists($file)) {
       $xml = file_get_contents($file);
       $arg1 = array("/function\-argument/","/operation\-argument/","/call\-arg\-name/","/analysis\-argument/","/call\-arg\-name/","/op\-name/","/analysis\-arg\-order/");
       $arg2 = array("function_argument","operation_argument","call_arg_name","analysis_argument","call_arg_name","op_name","analysis_arg_order");
       $xml = preg_replace($arg1,$arg2,$xml);
       $xml = simplexml_load_string($xml);
       
       $xml = $xml->functions;
        foreach ($xml->children() as $node) {
            $CALLS[] = new MPI_Calls($node);
        }
    }
    return $CALLS;
}


// PRINT varDump
function printvarDump($CALLS)
{
        echo "<pre>";
        var_dump($CALLS);
        echo "</pre>";
}


// PRINT function
function printFunctions($CALLS)
{
        echo "<table border='1'>
              <tr><td class=\"head\" ><b>Name:</b></td><td  class=\"head\" colspan='".GlobalInfo::$max_Arg_count."'><b>function arguments</b></td></tr>";
    $k=0;
    foreach($CALLS as $call)
    {
        if(!$call->isAllowed())
            continue;
        //set html class to design different line colors
        if($k%2)$class="class=\"l1\""; else $class="";
        $k++;
            
         echo"     <tr><td $class><b>".$call->getName()."</b></td>
        ";
        

        foreach($call->arguments as $arg)
        {
            echo "<td $class> ".$arg->getName()." </td>";
        }
        echo "</tr>";
    }
   echo " </table><br/><br/>";
}

// PRINT Todo
function printTodos($CALLS)
{
    $checksTodo = array();
    echo "<h3>TODO:</h3><a href=\"#List\" title=\"list\">List of todos</a><br/><table width=\"80%\">";
    foreach($CALLS as $call)
    {        
        $file_name = "todos/todo_".$call->name.".txt";
        if(is_file($file_name))
        {   
            echo "<tr><td width=\"150\"><b>".$call->name.":</td><td><pre>";
            $TODO = file($file_name);
            foreach($TODO as $val)
            {
                echo htmlspecialchars($val);
                $checks = explode("->",$val,2);
                if(count($checks)>1 && !in_array(trim($checks[1]),$checksTodo))$checksTodo[] = trim($checks[1]);
            }
             echo "</pre></td></tr><tr><td class=\"emptyRow\"><br/></td></tr>";
        }
    }
    echo "</table><br/>";
    echo "<h3>List of todos:</h3><a name=\"List\"></a><br><ul>";
    sort($checksTodo,SORT_STRING);
    foreach($checksTodo as $val)
        echo "<li>".htmlspecialchars($val)."</li>";
    echo "</u><br/><br/>";
}

// PRINT function with todos
function printFunctionsTodo($CALLS)
{
  if(!is_dir("todos"))
  {
        mkdir("todos",0777);
        
  }
        
    if(isset($_POST["send"]) && $_POST["send"] == "apply")
    {
        foreach($_POST as $key => $val)
        {
            if($key != "send")
            {
                if(is_file("todos/".$key.".txt"))
                {
                    unlink("todos/".$key.".txt");
                }
                 
                if(trim($val) != "")
                {
                    $fp = fopen ("todos/".$key.".txt", 'w' );
                    fputs ( $fp, $val );
                    fclose ( $fp );
                }
            }
        }
    }
    
    global $url;
    echo "<form action=\"index.php?view=6".$url."\" method=\"POST\">
          <table border='1'>
          <tr><td class=\"head\" ><b>Name:</b></td><td  class=\"head\" colspan='".GlobalInfo::$max_Arg_count."'><b>function arguments</b></td></tr>";
    $k=0;
    
    foreach($CALLS as $call)
    {
    $i=1;
        if(!$call->isAllowed())
            continue;
        //set html class to design different line colors
        if($k%2)$class="class=\"l1\""; else $class="";
        $k++;
            
         echo"     <tr><td $class><b>".$call->getName()."</b></td>
        ";
        
                
        foreach($call->arguments as $arg)
        {
            $i++;
            echo "<td $class> ".$arg->getName()." </td>";
        }
        echo "</tr>";
        $file_name = "todo_".$call->name;
        echo "<tr><td colspan=\"$i\"><textarea name=\"$file_name\" style=\"width:98%;\">";
        if(is_file("todos/".$file_name.".txt"))
        {
            $TODO = file("todos/".$file_name.".txt");
            foreach($TODO as $val)
                echo $val;
         }
        echo "</textarea></td></tr>";
    }
   echo " <tr><td><input type=\"submit\" name=\"send\" value=\"apply\" /></td></tr></table></form><br/><br/>";
}

// PRINT analyses mapped and todos
function printAnaTodos($CALLS)
{

  if(!is_dir("todos"))
  {
        mkdir("todos",0777);
        
  }
        
    if(isset($_POST["send"]) && $_POST["send"] == "apply")
    {
        foreach($_POST as $key => $val)
        {
            if($key != "send")
            {
                if(is_file("todos/".$key.".txt"))
                {
                    unlink("todos/".$key.".txt");
                }
                 
                if(trim($val) != "")
                {
                    $fp = fopen ("todos/".$key.".txt", 'w' );
                    fputs ( $fp, $val );
                    fclose ( $fp );
                }
            }
        }
    }
    
    global $url;
    echo "<form action=\"index.php?view=8".$url."\" method=\"POST\">";
        echo "<table border='1'>
              <tr><td class=\"head\"><b>Name:</b></td><td class=\"head\" colspan='".GlobalInfo::$max_Arg_count."'><b>arguments</b></td></tr>";
     $k=0;
    foreach($CALLS as $call)
    {
        $i=0;
        if(!$call->isAllowed()) continue;
        //set html class to design different line colors
        if($k%2)$class="class=\"l1\""; else $class="";
        $k++;

         echo"     <tr><td $class><b>".$call->getName()."</b></td>
        ";
        
        // PRINT MPI_ CALL          
        foreach($call->arguments as $arg)
        {
            echo "<td $class>".$arg->getName()."</td>";
        }
        echo "</tr>";
        
        // PRINT Analyses
        foreach($call->analyses as $ana)
        {
             if(!$ana->isAllowed())
                continue;
            echo "<tr><td $class>".$ana->getName()."</td>";
            foreach($call->arguments as $arg)
            {
                if($ana->isMappedToArgument($arg->name))
                        echo "<td $class><b>".$arg->getName()."</b></td>";
                else
                        echo "<td $class>&nbsp;<!--".$arg->getName()."--></td>";
            }
            echo "</tr>";
        }
        $file_name = "todo_".$call->name;
        echo "<tr><td colspan=\"$i\"><textarea name=\"$file_name\" style=\"width:98%;\">";
        if(is_file("todos/".$file_name.".txt"))
        {
            $TODO = file("todos/".$file_name.".txt");
            foreach($TODO as $val)
                echo $val;
         }
        echo "</textarea></td></tr>";

    }
    echo " <tr><td><input type=\"submit\" name=\"send\" value=\"apply\" /></td></tr></table></form><br/><br/>";
}

// PRINT analyses
function printAnalyses($CALLS)
{
        echo "<table border='1'>
              <tr><td class=\"head\"><b>Name:</b></td><td class=\"head\" colspan='".GlobalInfo::$max_analyses_count."'><b>analyses</b></td></tr>";
     $k=0;
    foreach($CALLS as $call)
    {
        if(!$call->isAllowed()) continue;
        //set html class to design different line colors
        if($k%2)$class="class=\"l1\""; else $class="";
        $k++;

         echo"     <tr><td $class><b>".$call->getName()."</b></td>
        ";
        
                
        foreach($call->analyses as $arg)
        {
            if(!$arg->isAllowed())
                continue;
                
            echo "<td $class>".$arg->getName()."</td>";
        }
        echo "</tr>";
    }
   echo " </table><br/><br/>";
}


// PRINT operations
function printOperation($CALLS)
{
        echo "<table border='1'>
              <tr><td class=\"head\"><b>Name:</b></td><td class=\"head\" colspan='".GlobalInfo::$max_operation_count."'><b>operation</b></td></tr>";
     $k=0;
    foreach($CALLS as $call)
    {
        if(!$call->isAllowed()) continue;
        //set html class to design different line colors
        if($k%2)$class="class=\"l1\""; else $class="";
        $k++;

         echo"     <tr><td $class><b>".$call->getName()."</b></td>
        ";
        
                
        foreach($call->operations as $opp)
        {
            echo "<td $class>".$opp->getName()."</td>";
        }
        echo "</tr>";
    }
   echo " </table><br/><br/>";
}


// PRINT analyses mapped
function printAnalysesMapped($CALLS)
{
        echo "<table border='1'>
              <tr><td class=\"head\"><b>Name:</b></td><td class=\"head\" colspan='".GlobalInfo::$max_Arg_count."'><b>arguments</b></td></tr>";
     $k=0;
    foreach($CALLS as $call)
    {
        if(!$call->isAllowed()) continue;
        if(count($call->analyses) == 0) continue;
        //set html class to design different line colors
        if($k%2)$class="class=\"l1\""; else $class="";
        $k++;

         echo"     <tr><td $class><b>".$call->getName()."</b></td>
        ";
        
        // PRINT MPI_ CALL          
        foreach($call->arguments as $arg)
        {
            echo "<td $class>".$arg->getName()."</td>";
        }
        echo "</tr>";
        
        // PRINT Analyses
        foreach($call->analyses as $ana)
        {
             if(!$ana->isAllowed())
                continue;
            echo "<tr><td $class>".$ana->getName()."</td>";
            foreach($call->arguments as $arg)
            {
                if($ana->isMappedToArgument($arg->name))
                        echo "<td $class><b>".$arg->getName()."</b></td>";
                else
                        echo "<td $class>&nbsp;<!--".$arg->getName()."--></td>";
            }
            echo "</tr>";
        }
        

    }
   echo " </table><br/><br/>";
}

// PRINT operation mapped
function printOperationMapped($CALLS)
{
        echo "<table border='1'>
              <tr><td class=\"head\"><b>Name:</b></td><td class=\"head\" colspan='".GlobalInfo::$max_Arg_count."'><b>arguments</b></td></tr>";
     $k=0;
    foreach($CALLS as $call)
    {
        if(!$call->isAllowed()) continue;
        //set html class to design different line colors
        if($k%2)$class="class=\"l1\""; else $class="";
        $k++;

         echo"     <tr><td $class><b>".$call->getName()."</b></td>
        ";
        
        // PRINT MPI_ CALL          
        foreach($call->arguments as $arg)
        {
            echo "<td $class>".$arg->getName()."</td>";
        }
        echo "</tr>";
        
        // PRINT operations
        foreach($call->operations as $ana)
        {
            echo "<tr><td $class>".$ana->getName()."</td>";
            foreach($call->arguments as $arg)
            {
           
                if($ana->isMappedToArgument($arg->name))
                        echo "<td $class><b>".$arg->getName()."</b></td>";
                else
                        echo "<td $class>&nbsp;<!--".$arg->getName()."--></td>";
            }
            echo "</tr>";
        }
        

    }
   echo " </table><br/><br/>";
}

function printCoustomizeView($CALLS)
{
     global $url;
    isset($_GET["sat"])?$sat="checked=checked":$sat=""; //show argument type
    isset($_GET["sai"])?$sai="checked=checked":$sai=""; //show argument intent
    isset($_GET["sao"])?$sao="checked=checked":$sao=""; //show analyses order
    isset($_GET["sag"])?$sag="checked=checked":$sag=""; //show analyses group
    isset($_GET["sano"])?$sano="checked=checked":$sano=""; //show analyses order
    isset($_GET["hl"])?$hl="checked=checked":$hl=""; //highlight filter
    isset($_GET["ft"])?$ft=$_GET["ft"]:$ft="";
    isset($_GET["fn"])?$fn=$_GET["fn"]:$fn="";
    isset($_GET["fc"])?$fc=$_GET["fc"]:$fc="";
    isset($_GET["fan"])?$fan=$_GET["fan"]:$fan="";
    isset($_GET["fac"])?$fac=$_GET["fac"]:$fac="";
      
    echo '<form action="index.php" method="GET">
           <input type="hidden" name="view" value="c" />
        <table>
        <tr><td><b>visibilities:</b></td></tr>
        <tr><td>show argument types</td><td><input type="checkbox" name="sat" value="1" '.$sat.'/></td></tr>
        <tr><td>show argument intent</td><td><input type="checkbox" name="sai" value="1" '.$sai.'/></td></tr>
        <tr><td>show argument order</td><td><input type="checkbox" name="sao" value="1" '.$sao.'/></td></tr>
        <tr><td>show analyses group</td><td><input type="checkbox" name="sag" value="1" '.$sag.' /></td></tr>
        <tr><td>show analyses order</td><td><input type="checkbox" name="sano" value="1" '.$sano.' /></td></tr>
        <tr><td>highlight if filter matches</td><td><input type="checkbox" name="hl" value="1" '.$hl.' /></td></tr>
        <tr><td><input type="submit" name="s" value="apply"/></td><td></td></tr>
        </table>
        <br/><br/>

        <table>
        <tr><td><b>filter calls:</b></td></tr>
        <tr><td colspan="2">
            <i>Notice: You can use this expressions to coustomize your filter:<br/>
               &quot;|&quot; matches either the part on the left side, or the part on the right side.<br/>
               &quot;.&quot; matches any single character.<br/>
               &quot;*&quot; 	Repeats the previous item zero or more times.
                
         </i></td></tr>
        <tr><td>by arg type:</td><td><input type="text" name="ft" value="'.$ft.'"/></td></tr>
        <tr><td>by arg name:</td><td><input type="text" name="fn" value="'.$fn.'" /></td></tr>
        <tr><td>by arg name:</td><td><select name="fin">
        <option value="all" ';
        if((isset($_GET["fin"]) && $_GET["fin"] == "all") || !isset($_GET["fin"])) echo "selected=selected ";
        echo '>all</option><option value="in" ';
        if(isset($_GET["fin"]) && $_GET["fin"] == "in") echo "selected=selected ";
        echo '>in and in/out</option><option value="out" ';
        if(isset($_GET["fin"]) && $_GET["fin"] == "out") echo "selected=selected ";
        echo '>out and in/out</option><option value="inout" ';
        if(isset($_GET["fin"]) && $_GET["fin"] == "inout") echo "selected=selected ";
        echo '>inout</option>
        </select></td></tr>
        <tr><td>by call name:</td><td><input type="text" name="fc" value="'.$fc.'" /></td></tr>
        <tr><td><input type="submit" name="s" value="apply"/></td><td></td></tr>
        </table><br/><br/>
        <br/><br/>
        
        <table>
        <tr><td><b>filter analyses:</b></td></tr>
        <tr><td colspan="2">
            <i>Notice: You can use this expressions to coustomize your filter:<br/>
               &quot;|&quot; matches either the part on the left side, or the part on the right side.<br/>
               &quot;.&quot; matches any single character.<br/>
               &quot;*&quot; 	Repeats the previous item zero or more times.
                
         </i></td></tr>
        <tr><td>by mapped arg name:</td><td><input type="text" name="fan" value="'.$fan.'" /></td></tr>
        <tr><td>by analyses name:</td><td><input type="text" name="fac" value="'.$fac.'" /></td></tr>
        <tr><td><input type="submit" name="s" value="apply"/></td><td></td></tr>
        </table><br/><br/>
        
        <br/><br/></form>
    ';
}
//=================
//= Klassen
//=================

class GlobalInfo
{
    public static $max_Arg_count = 0;
    public static $max_analyses_count = 0;
    public static $max_operation_count = 0;
}
class MPI_Calls 
{
  var $name;
  var $return_type;
  var $wrapp_everywhere;
  var $is_finalizer;
  var $arguments= array();
  var $analyses=array();
  var $operations=array();
  
  function MPI_Calls($xml)
  {

    $attr = $xml->attributes();
    $this->name =  sprintf("%s", $attr["name"]);
    $this->return_type =  sprintf("%s", $attr["return-type"]);

    $this->wrapp_everywhere =  sprintf("%s", $attr["wrapp-everywhere"]);
    if($this->wrapp_everywhere == "") $this->wrapp_everywhere = "no";
    $this->is_finalizer =  sprintf("%s", $attr["is-finalizer"]);
    if($this->is_finalizer == "") $this->is_finalizer = "no";
    
    if(isset($xml->function_arguments))
    {
        $count = count($xml->function_arguments->children());  
        GlobalInfo::$max_Arg_count = max(GlobalInfo::$max_Arg_count,$count);
        foreach($xml->function_arguments->children() as $arguments)
            $this->arguments[] = new arguments($arguments);
    }
    if(isset($xml->operations))
    {
        $count = count($xml->operations->children());  
        GlobalInfo::$max_operation_count = max(GlobalInfo::$max_operation_count,$count);          
        foreach($xml->operations->children() as $operation)
             $this->operations[] = new operation($operation);
    }
    if(isset($xml->analyses))
    {
        $count = count($xml->analyses->children());  
        GlobalInfo::$max_analyses_count = max(GlobalInfo::$max_analyses_count,$count);
        foreach($xml->analyses->children() as $analyses)
             $this->analyses[] = new analysis($analyses,$this->operations);
    }
  }
  
  public function isAllowed()
  {
    if(isset($_GET["ft"]) || isset($_GET["fn"]) || isset($_GET["fc"]) || isset($_GET["fin"]))
    {

        $out = array(false,false,false,false);
    //filter set
        if(isset($_GET["fc"]) && eregi($_GET["fc"],trim($this->name)))
             $out[0] = true;
       
       
        foreach($this->arguments as $arg)
        {
          //filter by argument name
          if(isset($_GET["fn"]) && !$out[2])
          {
            $type = trim($arg->name);
            if(eregi($_GET["fn"],$type))
                $out[2] = true;
          }
           
          //filter by argument type
          isset($_GET["fin"])?$intent=$_GET["fin"]:$intent = "all";
          if(isset($_GET["ft"]) && !$out[1])
          {
           if(eregi($intent,$arg->intent) || $intent=="all")
            {
               $type = ereg_replace("\*","",$arg->type);
               if(eregi($_GET["ft"],$type))
                  $out[1] = true;  
            }
          }

            //filter by intent
           if(eregi($intent,$arg->intent) || $intent=="all")
           {
                $out[3] = true;
            }

        }
         if((!isset($_GET["fc"]) || $out[0]) && (!isset($_GET["ft"]) || $out[1]) && (!isset($_GET["fn"]) || $out[2]) && (!isset($_GET["fin"]) || $out[3]) )
            return true;
          
        return false;
     }
     //filter not set
     return true;
  }
  public function getName()
  {
       $out = $this->name;
       if(isset($_GET["hl"]) && isset($_GET["fc"]) && eregi($_GET["fc"],trim($this->name)))
           $out = "<div class=\"marked\">".$out."</div>";
       return $out;
  }

}

class arguments
{
  var $name;
  var $type;
  var $intent;
  var $order;
  var $typeAfterArg;
  var $length_argument;
  var $operation;
  
  function arguments($xml)
  {
      $attr = $xml->attributes();
      $this->name =  sprintf("%s", $attr["name"]);
      $this->type =  sprintf("%s", $attr["type"]);
      $this->intent =  sprintf("%s", $attr["intent"]);
      $this->order =  sprintf("%s", $attr["order"]);
      $this->typeAfterArg =  sprintf("%s", $attr["typeAfterArg"]);
  }
  
  public function getName()
  {
       $out = $this->name;
       if(isset($_GET["sat"]))
            $out = $this->type." ".$out.$this->typeAfterArg;
      
       if(isset($_GET["sai"]))
            $out = $out." (".$this->intent.")";
       
       if(isset($_GET["sao"]))
            $out = $this->order.": ".$out;
       

       if(isset($_GET["hl"]) && 
         (
           (isset($_GET["ft"]) && eregi($_GET["ft"],trim($this->type))) ||
           (isset($_GET["fn"]) && eregi($_GET["fn"],trim($this->name)))
         )
         )
           $out = "<div class=\"marked\">".$out."</div>";
       
       return $out;
  }
}

class analysis
{
  var $name;
  var $order;
  var $group;
  var $arguments=array();
  function analysis($xml,$operation)
  {
    $attr = $xml->attributes();
    $this->name =  sprintf("%s", $attr["name"]);
    $this->order =  sprintf("%s", $attr["order"]);
    $this->group =  sprintf("%s", $attr["group"]);
    if(isset($xml->analysis_arguments))
        foreach($xml->analysis_arguments->children() as $arguments)
            $this->arguments[] = new analyse_arguments($arguments,$operation);
    
  }

  public function isMappedToArgument($argument)
  {

    foreach($this->arguments as $arg)
    {
            if($arg->argument_mapping == $argument)
            {
                return true;
            }

            if(isset($arg->operation) && $arg->operation->isMappedToArgument($argument))
            {
                return true;
            }

    }
    return false;
  }
  
  public function isAllowed()
  {
    if(isset($_GET["fat"]) || isset($_GET["fan"]) || isset($_GET["fac"]))
    {
    //filter set
        if(isset($_GET["fac"]) && eregi($_GET["fac"],trim($this->name) ))
            return true;
            
        foreach($this->arguments as $arg)
        {
          //filter by argument name
          if(isset($_GET["fan"]))
          foreach($this->arguments as $arg)
          {
            if(eregi($_GET["fan"],$arg->argument_mapping))
            {
                return true;
            }

            if(isset($arg->operation) && $arg->operation->isMappedToArgument($_GET["fan"]))
            {
                return true;
            }
           }
        }
        
        return false;
     }
     //filter not set
     return true;
  }

  public function getName()
  {
      $out = $this->name;
      if(!isset($_GET["sag"]))
        $out = substr(stristr($this->name,":"),1);  
      
      if(isset($_GET["sano"]))
        $out .= "(".$this->order.")";  
     
      return $out;
  }
}

class analyse_arguments
{
    var $type;
    var $analysis_arg_order;
    var $operation;
    var $argument_mapping;
    function analyse_arguments($xml,$operations)
    {
        $attr =  $xml->attributes();
        $this->type =  sprintf("%s", $attr["type"]);
        $this->analysis_arg_order =  sprintf("%s", $attr["analysis_arg_order"]);
        if(isset($xml->call_arg_name))
            $this->argument_mapping = sprintf("%s", $xml->call_arg_name);
        if(isset($xml->op_name))
        {
            $group = sprintf("%s", $xml->op_name->attributes()->group);
            $id = sprintf("%s", $xml->op_name->attributes()->id);
            $name = sprintf("%s", $xml->op_name);
            
            foreach($operations as $operation)
            {

                if($operation->name == $name && 
                   $operation->group == $group && 
                   $operation->id == $id 
                  )
                {
                    $this->operation = $operation;
                    break;
                }
            }
        }
    }
}

class operation
{
  var $name;
  var $group;
  var $order;
  var $id;
  var $operation_arguments = array();
  function operation($xml)
  {   
      $attr = $xml->attributes();
      $this->order =  sprintf("%s", $attr["order"]);
      $this->name =  sprintf("%s", $attr["name"]);
      $this->group =  sprintf("%s", $attr["group"]);
      $this->id =  sprintf("%s", $attr["id"]);
      if(isset($xml->operation_arguments))   
          foreach($xml->operation_arguments->children() as $args)
          {
              $this->operation_arguments[] = new operation_arguments($args);
           }
  }

  public function isMappedToArgument($argument)
  {
    $out = false;
    foreach($this->operation_arguments as $arg)
    {
        if($arg->call_arg_name == $argument)
        {
            $out = true;
            break;
        }
    }
    return $out;
  }
  
  public function getName()
  {
       return $this->name;
  }
}

class operation_arguments
{
    var $op_arg_order;
    var $call_arg_name;
    function operation_arguments($xml)
    {
      $attr = $xml->attributes();
      $this->op_arg_order =  sprintf("%s", $attr["op-arg-order"]);
      $this->call_arg_name =  sprintf("%s", $xml->call_arg_name[0]);
    }
}
?>
</body></html>
