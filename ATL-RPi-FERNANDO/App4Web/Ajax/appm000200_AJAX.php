<?php
   class appm000200_AJAX extends Controller {
	   public $_system; 

       public function init($_start) {
          // Um INIT nao herda o anterior da classe da qual foi derivado, cada um executa apenas o seu
          //Identificacao do Tela a ser processada
		  $this->_system = $_start;
		  		   
	   }
	   
       public function index_action() {
       	   global $start;
		   //-- Chegou ate aqui, pode aqbrir a view, ufa

		   //-- Identificacao
		   $MODULO="index";
		   $LINK_MODULO = "{$this->_system->_link}appm000200/index/{$MODULO}/";
		   
		   //-- Identificacao do Tela a ser processada
		   $this->_system->_dados['tel_nome']           = "Ajax appm000200";
		   $this->_system->_dados['tel_titulo']         = "Acionadores";
		   $this->_system->_dados['tel_id']             = "0";
		   $this->_system->_dados['tel_cod']            = "appm000200";
		   
		   // Include padrao verificacoes basicas
		   //include("Include/portalInclude.php");
		   
		   //-- Acionando o PHP por detras do AJAX/JSON o _JSON poe automatico cuidado :|
		   $this->loadJSON('appm000200',$this->_system->_dados);
	   }	
	   
	   public function ajax() {
	       $this->index_action();
	   }
   }
?>