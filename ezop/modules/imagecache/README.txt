Imagecache is a dynamic image manipulation and cache tool.
It allows you to create a namespace that corresponds to a set
of image manipulation actions. It generates a derivative image the
first time it is requested from a namespace until the namespace or
the entire imagecache is flushed.


Usage:
goto  Administer -> Site Configuration -> Image cache 
create a ruleset,
add some actions to yoru ruleset,

add a 
print theme('imagecache', $ruleset_namespace, $image['filepath'], $alt, $title, $attributes)
to your tpl.php file where you would like the image to appear, where $alt, $title and $attributes are optional parameters.





