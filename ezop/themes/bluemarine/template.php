<?php

function phptemplate_user_profile($user, $fields = array())
{
	return _phptemplate_callback('user_profile', array('user' => $user, 'fields' => $fields));
}

?>