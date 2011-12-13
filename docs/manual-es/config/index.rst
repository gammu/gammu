.. _gammurc:

Archivo de configuración de Gammu
==================================

Sinopsis
--------

En Linux, MacOS X, BSD y otros sistemas similares a Unix, se buscará por
archivos de configuración en el siguiente órden:

1. ``$XDG_CONFIG_HOME/gammu/config``
2. ``~/.config/gammu/config``
3. ``~/.gammurc``
4. ``/etc/gammurc``

En Microsoft Windows:

1. ``$PROFILE\Application Data\gammurc``
2. ``.\gammurc``

Descripción
------------

Gammu necesita configuración para poder comunicarse apropiadamente con el
teléfono.
:ref:`gammu` lee su configuración de un archivo. Su ubicación es determinada
en tiempo de ejecució, revisa más arriba para las rutas de búsqueda.

Puede utilizar :ref:`gammu-config` o :ref:`gammu-detect` para generar un
archivo de configuración o comienza con un :ref:`ejemplo bien documentado`.

Para ayudas sobre la configuración de un teléfono, puede revisar la base de
datos de teléfonos de Gammu («Gammu Phone Database»)
<http://wammu.eu/phones/> para conocer sobre la experiencia de otros
usuarios.

Este archivo utiliza la sintáxis de archivos ini, revisa :ref:`ini`.

Un archivo de configuración para gammu puede contener varias secciones:
``[gammu]``, ``[gammu1]``, ``[gammuN]``, etc. Cada sección determina una
configuración de conexión y en el modo predeterminado gammu intentará con
todas en órden numérico. También puede especificar la sección de
configuración a utilizar proveyendo su número (:config:section:`[gammu]` es
la número 0) como parámetro a :ref:`gammu`; y éste sólo utilizará dicha
sección.

.. config:section:: [gammu]

Esta sección es leída de forma predeterminada a menos que se especifique lo
contrario mediante la línea de órdenes.

Parámetros de conexión al dispositivo
+++++++++++++++++++++++++++++++++++++++

.. config:option:: Conexión

    El protocolo a utilizar para comunicarse con el teléfono.

    Para los cables Nokia deseará utilizar uno de los siguientes:

    ``fbus``
        conexión serial FBUS
    ``dlr3``
        DLR-3 y cables compatibles
    ``dku2``
        DKU-2 y cables compatibles
    ``dku5``
        DKU-5 y cables compatibles
    ``mbus``
        conexión MBUS serial

    Si utiliza algún cable no original, probablemente necesite agregar
    ``-nodtr`` (por ejemplo para cables basados en ARK3116) o ``-nopower``, pero
    Gammu debería de poder detectarlo automáticamente.

    Para teléfonos conectados con un cable de fabricantes distintos a Nokia,
    generalmente deseará utilizar:

    ``at``
        conexión genérica basada en órdenes AT

    Opcionalmente se puede especificar la velocidad de conexión, por ejemplo
    ``at19200``, pero no es necesario para cables USB modernos.

    Para conexiones IrDA utilice uno de los siguientes:

    ``irdaphonet``
        Conexión Phonet para teléfonos Nokia.
    ``irdaat``
        Conexión de órdenes AT para la mayoría de los teléfonos (no funcionará en
        Linux).
    ``idraobex``
        Conexión OBEX (IrMC o transferencia de archivos) para la mayoría de los
        teléfonos.
    ``irdagnapbus``
        Conexión basada en GNapplet para teléfonos Symbian, revisa :ref:`gnapplet`.

    Para conexiones Bluetooth, utiliza uno de los siguientes:

    ``bluephonet``
        Conexión Phonet para teléfonos Nokia.
    ``blueat``
        Conexión de órdenes AT para la mayoría de los teléfonos.
    ``blueobex``
        Conexión OBEX (IrMC o transferencia de archivos) para la mayoría de los
        teléfonos.
    ``bluerfgnapbus``
        Conexión basada en GNapplet para teléfonos Symbian, revisa :ref:`gnapplet`.
    ``blues60``
        Conexión a la miniaplicación Series60 en teléfonos S60, revisa :ref:`s60`.

        .. versionadded::         1.29.90

    .. seealso::     :ref:`faq-config`

.. config:option:: Dispositivo

    .. versionadded::     1.27.95

    Nodo del dispositivo o dirección del teléfono. Depende de la conexión
    utilizada.

    Para **cables** o puertos seriales emulados, ingrese el nombre del
    dispositivo (por ejemplo ``/dev/ttyS0``, ``/dev/ttyACM0``, ``/dev/ircomm0``,
    ``/dev/rfcomm0`` en Linux, ``/dev/cuad0`` en FreeBSD ó ``COM1:`` en
    Windows). Las excepciones son los cables DKU-2 y DKU-5 en Windows, donde el
    dispositivo es detectado automáticamente utilizando información del
    controlador y este parámetro es ignorado.

    Para conexiones **USB** (actualmente sólo fbususb y dku2 en Linux), puede
    especificar el dispositivo USB al que Gammu se debe conectar. Puede proveer
    alguno de:
    ID de fabricante/producto o la dirección de dispositivo en USB::

        Device = 0x1234:0x5678 # Buscar dispositivo por ID de fabricante y producto
        Device = 0x1234:-1 # Buscar dispositivo por ID de fabricante
        Device = 1.10 # Buscar dispositivo por bus USB y dirección de dispositivo
        Device = 10 # Buscar dispositivo por dirección de dispositivo USB
        Device = serial:123456 # Buscar dispositivo por cadena de serial

    .. note::

        En sistemas Linux puede no tener permisos sobre algunos nodos de
        dispositivos. Podría necesitar ser miembro de algún grupo (por ejemplo
        :samp:`plugdev` o :samp:`dialout`) o agregar reglas especiales a udev para
        permitir el acceso a estos dispositivos a los usuarios distintos a root.

        Para teléfonos Nokia puede agregar el archivo siguiente (también disponible
        en el código fuente como :file:`contrib/udev/45-nokiadku2.rules`) en
        :file:`/etc/udev/rules.d/45-nokiadku2.rules`:

        .. literalinclude::         ../../../contrib/udev/45-nokiadku2.rules
           :language: sh

    En caso que el dispositivo USB aparezca como un puerto serial en el sistema
    (por ejemplo ``/dev/ttyACM0`` en Linux o ``COM5:`` en Windows), simplemente
    utilice la misma configuración para puerto serial.

    Para conexiones **Bluetooth** debe ingresar la dirección Bluetooth del
    teléfono (en Linux puede ver una lista de dispositivos Bluetooth en rango
    utilizando la órden :command:`hcitool scan`). Opcionalmente, puede forzar
    Gammu a utilizar un canal específico incluyendo su número luego de la barra
    oblicua.

    Antes de utilizar Gammu, el dispositivo debería de estar emparejado con el
    equipo o debe configurar emparejamiento automático.

    Para conexiones **IrDA** estos parámetros no son utilizados en absoluto.

    Si IrDA no funciona en Linux, puede llegar a necesitar activar la interfaz y
    activar desubrimiento (necesitará ejecutar las órdenes siguientes como
    root):

    .. code-block::     sh

        ip l s dev irda0 up # Activa el dispositivo irda0
        sysctl net.irda.discovery=1 # Activa descubrimiento de dispositivos sobre IrDA

    .. note::

        IrDA nativo no funcionará en Linux, necesitará configurar un puerto serial
        virtual para el mismo (por ejemplo ``/dev/ircomm0``) y utilizarlo al igual
        que un cable. Esto puede lograrse generalmente cargando los móludos
        ``ircomm-tty`` y ``irtty-sir``:

        .. code-block::         sh

            modprobe ircomm-tty modprobe irtty-sir

    .. seealso::     :ref:`faq-config`

.. config:option:: Puerto

    .. deprecated::     1.27.95

    Alias para :config:option:`Device`, mantenido sólo por cuestiones de
    compatibilidad.

.. config:option:: Modelo

    ¡No utilice este parámetro a menos que sea realmente necesario! El único
    caso de uso para este es cuando Gammu no reconoce el teléfono y detecta
    errónamente sus funcionalidades.

    El único caso especias para utilizar el modelo es forzar el tipo de conexión
    especial OBEX en lugar de dejar que Gammu intente la mejor opción para la
    operación seleccionada:

    ``obexfs``
        forzar la utilización del servicio de navegación de archivos (soporte de
        sistema de archivos)
    ``obexirmc``
        forzar la utilización del servicio IrMC (soporte de contactos, calendario y
        notas)
    ``obexnone``
        no se elija ningún servicio, esto sólo tiene una utilidad limitada para
        enviar archivos (la órden :option:`gammu sendfile`)
    ``mobex``
        servicio m-obex para teléfonos Samsung

.. config:option:: Use_Locking

    En sistemas Posix, podría desear bloquear el dispositivo serial utilizando
    archivos de bloqueo estilo UUCP cuando está siendo utilizado. Activar esta
    opción (definiéndola como «yes») hará que Gammu honre estos bloqueos y los
    cree al iniciar. En la mayoría de las distribuciones necesitará privilegios
    adicionales para utilizar estos bloqueos (por ejemplo, necesitará ser
    miembro del grupo uucp).

    Esta opción no tiene efectos en Windows.

Opciones de conexión
+++++++++++++++++++++

.. config:option:: SyncronizeTime

    Si desea definir la hora del teléfono desde el equipo durante el comienzo de
    la conexión.

.. config:option:: StartInfo

    Esta opción permite (definiéndola como «yes») ver un mensaje en la pantalla
    del teléfono o encender la luz del mismo por un momento durante el comienzo
    de la conexión. El teléfono no sonará durante el comienzo de la conexión con
    esta opción. Esto sólo funciona con algúnos teléfonos Nokia.


Opciones de depuración
+++++++++++++++++++++++

.. config:option:: LogFile

    Ruta del archivo donde se almacenará información sobre la comunicación.

    .. note::

        Para la mayoría de los niveles de depuración (excluyendo ``errors``) el
        archivo de registros es reemplazado en cada ejecución.

.. config:option:: LogFormat

    Determina qué será agregado al registro definido por
    ::config:option:`LogFile`. Los valores posibles son:

    ``nothing``
        ningún nivel de depuración
    ``text``
        un volcado de la transmisión en formato de texto
    ``textall``
        toda información posible en formato de texto
    ``textalldate``
        toda la información posible en formato de texto, con marcas temporales
    ``errors``
        errores en formato de texto
    ``errorsdate``
        errores en formato de texto, con marcas temporales
    ``binary``
        volcado de la transmisión en formato binario

    Para depuración, use ``textalldate`` o ``textall``, ya que contienen toda la
    información necesaria para diagnosticar problemas.

.. config:option:: Funcionalidades

    Características particulares del teléfono. Esto puede ser utilizado como
    reemplazo cuando los valores codificados en ``common/gsmphones.c`` no son
    correctos o no existen. Consulte ``include/gammu-info.h`` por todos los
    posibles valores (todos los valores :c:type:`GSM_Feature` que no comiencen
    con el prefijo ``F_``). Porfavor, reporta los valores correctos a los
    autores de Gammu.

Opciones de conjunto de caracteres y localización
++++++++++++++++++++++++++++++++++++++++++++++++++

.. config:option:: GammuCoding

    Fuerza el uso de una página de códigos específica (por ejemplo ``1250``
    forzará CP-1250 ó ``utf8`` para UTF-8). Esto no debería ser necesario, Gammu
    lo detecta dependiendo de su configuración de localización.

.. config:option:: GammuLoc

    La ruta al directorio con los archivos de localización (el directorio debe
    contener ``LANG/LC_MESSAGES/gammu.mo``). Si gammu está instalado
    correctamente debería encontrar estos archivos automáticamente.

Otras opciones
++++++++++++++

.. config:option:: DataPath

    Rutas adicionales donde buscar por archivos de datos. La ruta predeterminada
    es configurada en tiempo de compilación (con el valor
    ``/usr/share/data/gammu`` en sistemas Unix). Actualmente es sólo utilizada
    para buscar archivos que cargar al teléfono utilizando :option:`gammu
    install`

Ejemplos
--------

Existe un ejemplo más completo en la documentación de Gammu, revise
:ref:`gammu`.

Exemplo de conexión
++++++++++++++++++++

Configuración de Gammu para un teléfono Nokia utilizando un cable DLR-3:

.. code-block:: ini

    [gammu] device = /dev/ttyACM0 connection = dlr3

Configuración de Gammu para un teléfono Sony-Ericsson (o cualquier otro
teléfono compatible con AT) conectado utilizando un cable USB:

.. code-block:: ini

    [gammu] device = /dev/ttyACM0 connection = at

Configuración de Gammu para un teléfono Sony-Ericsson (o cualquier otro
teléfono compatible con AT) conectado utilizando bluetooth:

.. code-block:: ini

    [gammu] device = B0:0B:00:00:FA:CE connection = blueat

Configuración de Gammy para un teléfono que necesita un ajuste manual de
Bluetooth para forzar el uso del canal 42:

.. code-block:: ini

    [gammu] device = B0:0B:00:00:FA:CE/42 connection = blueat

Trabajando con múltiples teléfonos
++++++++++++++++++++++++++++++++++++

Gammu puede ser configurado para el uso de múltiples teléfonos (sin embargo
no se utiliza más de una conexión simultáneamente, puede elegir aquella a
utilizar con el parámetro :option:`gammu -s`). La configuración de teléfonos
en tres puertos seriales se vería como sigue:

.. code-block:: ini

    [gammu] device = /dev/ttyS0 connection = at

    [gammmu1] device = /dev/ttyS1 connection = at

    [gammmu2] device = /dev/ttyS2 connection = at

.. _Fully documented example:

Ejemplo completamente documentado
+++++++++++++++++++++++++++++++++

Puede encontrar este archivo de ejemplo como :fil:`docs/config/gammurc` en
el código fuente de Gammu.

.. literalinclude:: ../../../docs/config/gammurc
   :language: ini

