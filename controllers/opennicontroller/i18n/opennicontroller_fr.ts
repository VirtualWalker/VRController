<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr_FR">
<context>
    <name>OpenNIApplication</name>
    <message>
        <location filename="../src/openniapplication.cpp" line="102"/>
        <source>The application will assume that a kinect sensor is used !</source>
        <translation>L&apos;application assume qu&apos;une caméra Kinect est utilisée !</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="182"/>
        <source>OpenNI already initialized !</source>
        <translation>OpenNI est déjà initialisé !</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="200"/>
        <source>Initializing OpenNI ...</source>
        <translation>Initialisation d&apos;OpenNI ...</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="219"/>
        <source>Open XML file</source>
        <comment>on error</comment>
        <translation>Ouverture du fichier XML</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="226"/>
        <source>Find depth generator</source>
        <comment>on error</comment>
        <translation>Recherche du générateur de profondeur</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="234"/>
        <source>Find user generator</source>
        <comment>on error</comment>
        <translation>Recherche du générateur d&apos;utilisateur</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="239"/>
        <source>Register to user callbacks</source>
        <comment>on error</comment>
        <translation>Enregistrement du retour utilisateur</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="241"/>
        <source>Register to calibration start</source>
        <comment>on error</comment>
        <translation>Enregistrement du début de calibration</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="243"/>
        <source>Register to calibration complete</source>
        <comment>on error</comment>
        <translation>Enregistrement de réussite de calibration</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="245"/>
        <source>Register to calibration in progress</source>
        <comment>on error</comment>
        <translation>Enregistrement de calibration en cours</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="250"/>
        <source>Supplied user generator doesn&apos;t support skeleton capability.</source>
        <translation>Le générateur d&apos;utilisateurs ne supporte pas la reconnaissance du squelette.</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="256"/>
        <source>Pose calibration required but not supported by this program.</source>
        <translation>Une posture de calibration est requise mais cette fonctionnalité n&apos;est pas supportée.</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="271"/>
        <source>The application is not initilized, can&apos;t start !</source>
        <translation>L&apos;application n&apos;a pas été initialisée. Impossible de la démarer !</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="275"/>
        <source>Starting OpenNI main loop ...</source>
        <translation>Lancement de la boucle principale d&apos;OpenNI ...</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="278"/>
        <source>Start Generating</source>
        <comment>on error</comment>
        <translation>Début de la génération</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/openniapplication.cpp" line="32"/>
        <source>%1 failed: %2</source>
        <comment>%1 is what failed and %2 is the error from OpenNI SDK.</comment>
        <translation>Echec: %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="40"/>
        <source>Cannot get the OpenNI application.</source>
        <translation>Impossible d&apos;obtenir l&apos;application OpenNI.</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="51"/>
        <source>New user: %1</source>
        <comment>%1: user ID</comment>
        <translation>Nouvel utilisateur: %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="60"/>
        <source>Lost user: %1</source>
        <comment>%1: user ID</comment>
        <translation>Perte de l&apos;utilisateur %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="65"/>
        <source>Calibration started for user: %1</source>
        <comment>%1: user ID</comment>
        <translation>Début de la calibration pour l&apos;utilisateur %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="76"/>
        <source>Calibration complete, start tracking user %1</source>
        <comment>%1: user ID</comment>
        <translation>Calibration réussite, début du suivi de l&apos;utilisateur %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="82"/>
        <source>Calibration failed for user: %1</source>
        <comment>%1: user ID</comment>
        <translation>Echec de la calibration pour l&apos;utilisateur %1</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="84"/>
        <source>Manual abort occured, stop attempting to calibrate !</source>
        <translation>Un arrêt manuel a été fait, fin de la calibration !</translation>
    </message>
    <message>
        <location filename="../src/openniapplication.cpp" line="114"/>
        <source>Stopping the OpenNI frame loop ...</source>
        <translation>Arrêt de la boucle de récupération d&apos;images d&apos;OpenNI ...</translation>
    </message>
</context>
<context>
    <name>USBController</name>
    <message>
        <location filename="../src/usbcontroller.h" line="55"/>
        <source>The USB controller is already initialized !</source>
        <translation>Le controlleur USB est déjà initialisé !</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="63"/>
        <source>Cannot initialize the USB controller. Error code: %1</source>
        <translation>Impossible d&apos;initialiser le controlleur USB. Code d&apos;erreur: %1</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="70"/>
        <source>Cannot open the usb device (VID: %1, PID: %2). Error code: %3</source>
        <translation>Impossible d&apos;ouvrir le périphérique USB (VID: %1, PID: %2). Code d&apos;erreur: %3</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="74"/>
        <source>USB device initialized (VID: %1, PID: %2) !</source>
        <translation>Le périphérique USB est initialisé (VID: %1, PID: %2) !</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="84"/>
        <source>The USB controller is not initialized !</source>
        <translation>Le controlleur USB n&apos;a pas encore été initialisé !</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="92"/>
        <source>Error when sending data to the USB device.</source>
        <translation>Erreur lors de l&apos;envoi de données au périphérique USB.</translation>
    </message>
    <message>
        <location filename="../src/usbcontroller.h" line="131"/>
        <source>You must specify an angle between -30° and 30°.</source>
        <translation>Vous devez spécifier un angle entre -30° et 30°.</translation>
    </message>
</context>
</TS>
