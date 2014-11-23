<?php

$images = array('any1.gif',
                'any2.gif',
		'any3.gif',
		'77257.gif',
		'av1.gif');
?>

<script type="text/javascript">
    eval('var images1 = ' + '<?php echo json_encode($images); ?>');
	
    alert(images1[1]);
</script>

<script type="text/javascript">
alert(images1[0]);
</script>