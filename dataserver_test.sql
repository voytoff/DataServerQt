/*M!999999\- enable the sandbox mode */ 
-- MariaDB dump 10.19-11.7.2-MariaDB, for Win64 (AMD64)
--
-- Host: localhost    Database: dataserver_test
-- ------------------------------------------------------
-- Server version	12.3.2-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*M!100616 SET @OLD_NOTE_VERBOSITY=@@NOTE_VERBOSITY, NOTE_VERBOSITY=0 */;

--
-- Table structure for table `calibration`
--

DROP TABLE IF EXISTS `calibration`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `calibration` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `sensor_id` int(11) DEFAULT NULL,
  `sensor_type_id` int(11) DEFAULT NULL,
  `kind` int(11) NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_calibration_sensor_kind` (`sensor_id`,`kind`),
  UNIQUE KEY `uq_calibration_sensor_type_kind` (`sensor_type_id`,`kind`),
  CONSTRAINT `fk_calibration_sensor` FOREIGN KEY (`sensor_id`) REFERENCES `sensor` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `fk_calibration_sensor_type` FOREIGN KEY (`sensor_type_id`) REFERENCES `sensor_type` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `chk_calibration_kind` CHECK (`kind` in (0,1))
) ENGINE=InnoDB AUTO_INCREMENT=136 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `calibration`
--

LOCK TABLES `calibration` WRITE;
/*!40000 ALTER TABLE `calibration` DISABLE KEYS */;
INSERT INTO `calibration` VALUES
(1,NULL,1,0,'A: y = 0.1*x'),
(2,2,NULL,0,'B: y = x - 20'),
(79,2,NULL,1,'Created by Qt Test');
/*!40000 ALTER TABLE `calibration` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `calibration_coefficient`
--

DROP TABLE IF EXISTS `calibration_coefficient`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `calibration_coefficient` (
  `calibration_id` int(11) NOT NULL,
  `index` int(11) NOT NULL,
  `value` double NOT NULL,
  PRIMARY KEY (`calibration_id`,`index`),
  CONSTRAINT `fk_calibration_coefficient_calibration` FOREIGN KEY (`calibration_id`) REFERENCES `calibration` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `calibration_coefficient`
--

LOCK TABLES `calibration_coefficient` WRITE;
/*!40000 ALTER TABLE `calibration_coefficient` DISABLE KEYS */;
INSERT INTO `calibration_coefficient` VALUES
(79,0,2),
(79,1,3),
(79,2,4);
/*!40000 ALTER TABLE `calibration_coefficient` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `calibration_point`
--

DROP TABLE IF EXISTS `calibration_point`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `calibration_point` (
  `calibration_id` int(11) NOT NULL,
  `index` int(11) NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  PRIMARY KEY (`calibration_id`,`index`),
  CONSTRAINT `fk_calibration_point_calibration` FOREIGN KEY (`calibration_id`) REFERENCES `calibration` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `calibration_point`
--

LOCK TABLES `calibration_point` WRITE;
/*!40000 ALTER TABLE `calibration_point` DISABLE KEYS */;
INSERT INTO `calibration_point` VALUES
(1,0,0,0),
(1,1,100,10),
(2,0,0,-20),
(2,1,100,80);
/*!40000 ALTER TABLE `calibration_point` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configuration`
--

DROP TABLE IF EXISTS `configuration`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `configuration` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(128) NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  `udp_port` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_configuration_name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `configuration`
--

LOCK TABLES `configuration` WRITE;
/*!40000 ALTER TABLE `configuration` DISABLE KEYS */;
INSERT INTO `configuration` VALUES
(1,'Test experiment','Deterministic DataServer integration tests',5101);
/*!40000 ALTER TABLE `configuration` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configuration_module`
--

DROP TABLE IF EXISTS `configuration_module`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `configuration_module` (
  `configuration_id` int(11) NOT NULL,
  `module_id` int(11) NOT NULL,
  `active` tinyint(4) NOT NULL DEFAULT 1,
  `settings` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`configuration_id`,`module_id`),
  KEY `fk_configuration_module_module` (`module_id`),
  CONSTRAINT `fk_configuration_module_configuration` FOREIGN KEY (`configuration_id`) REFERENCES `configuration` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_configuration_module_module` FOREIGN KEY (`module_id`) REFERENCES `module` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `configuration_module`
--

LOCK TABLES `configuration_module` WRITE;
/*!40000 ALTER TABLE `configuration_module` DISABLE KEYS */;
INSERT INTO `configuration_module` VALUES
(1,1,1,'{\"channelRate\":1000,\"mode\":0}');
/*!40000 ALTER TABLE `configuration_module` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configuration_parameter`
--

DROP TABLE IF EXISTS `configuration_parameter`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `configuration_parameter` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `configuration_id` int(11) NOT NULL,
  `name` varchar(128) NOT NULL,
  `data_type` varchar(32) NOT NULL DEFAULT 'double',
  `value` text NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_configuration_parameter_name` (`configuration_id`,`name`),
  KEY `ix_configuration_parameter_configuration` (`configuration_id`),
  CONSTRAINT `fk_configuration_parameter_configuration` FOREIGN KEY (`configuration_id`) REFERENCES `configuration` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `configuration_parameter`
--

LOCK TABLES `configuration_parameter` WRITE;
/*!40000 ALTER TABLE `configuration_parameter` DISABLE KEYS */;
/*!40000 ALTER TABLE `configuration_parameter` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configuration_signal_definition`
--

DROP TABLE IF EXISTS `configuration_signal_definition`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `configuration_signal_definition` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `configuration_id` int(11) NOT NULL,
  `name` varchar(128) NOT NULL,
  `kind` int(11) NOT NULL,
  `tag_id` int(11) DEFAULT NULL,
  `signal_type_id` int(11) DEFAULT NULL,
  `formula` text DEFAULT NULL,
  `archive_rate` int(11) NOT NULL DEFAULT 10,
  `sensor_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_configuration_signal_name` (`configuration_id`,`name`),
  KEY `ix_signal_configuration` (`configuration_id`),
  KEY `ix_signal_tag` (`tag_id`),
  KEY `ix_signal_type` (`signal_type_id`),
  KEY `ix_signal_sensor` (`sensor_id`),
  CONSTRAINT `fk_signal_configuration` FOREIGN KEY (`configuration_id`) REFERENCES `configuration` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_signal_sensor` FOREIGN KEY (`sensor_id`) REFERENCES `sensor` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `fk_signal_tag` FOREIGN KEY (`tag_id`) REFERENCES `configuration_tag` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_signal_type` FOREIGN KEY (`signal_type_id`) REFERENCES `signal_type` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `chk_signal_kind` CHECK (`kind` in (0,1))
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `configuration_signal_definition`
--

LOCK TABLES `configuration_signal_definition` WRITE;
/*!40000 ALTER TABLE `configuration_signal_definition` DISABLE KEYS */;
INSERT INTO `configuration_signal_definition` VALUES
(3,1,'A',1,NULL,1,'Raw0',100,1),
(4,1,'B',1,NULL,2,'Raw1',10,2),
(5,1,'C',1,NULL,2,'A + B',10,NULL),
(6,1,'Raw0',0,11,1,NULL,1000,NULL),
(10,1,'Raw1',0,20,2,NULL,100,NULL);
/*!40000 ALTER TABLE `configuration_signal_definition` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configuration_tag`
--

DROP TABLE IF EXISTS `configuration_tag`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `configuration_tag` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `configuration_id` int(11) NOT NULL,
  `module_id` int(11) NOT NULL,
  `channel` int(11) NOT NULL,
  `active` tinyint(4) NOT NULL DEFAULT 1,
  `settings` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_configuration_tag_channel` (`configuration_id`,`module_id`,`channel`),
  KEY `ix_configuration_tag_configuration` (`configuration_id`),
  KEY `ix_configuration_tag_module` (`module_id`),
  CONSTRAINT `fk_configuration_tag_configuration` FOREIGN KEY (`configuration_id`) REFERENCES `configuration` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_configuration_tag_module` FOREIGN KEY (`configuration_id`, `module_id`) REFERENCES `configuration_module` (`configuration_id`, `module_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=50 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `configuration_tag`
--

LOCK TABLES `configuration_tag` WRITE;
/*!40000 ALTER TABLE `configuration_tag` DISABLE KEYS */;
INSERT INTO `configuration_tag` VALUES
(11,1,1,0,1,'{\"range\":1}'),
(20,1,1,1,1,'{\"range\":1}');
/*!40000 ALTER TABLE `configuration_tag` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `crate`
--

DROP TABLE IF EXISTS `crate`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `crate` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL,
  `serial` varchar(128) NOT NULL,
  `host` varchar(255) NOT NULL,
  `port` int(11) NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_crate_serial` (`serial`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `crate`
--

LOCK TABLES `crate` WRITE;
/*!40000 ALTER TABLE `crate` DISABLE KEYS */;
INSERT INTO `crate` VALUES
(1,1,'3T778029','127.0.0.1',11111,'CRATE16 test');
/*!40000 ALTER TABLE `crate` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `global_parameter`
--

DROP TABLE IF EXISTS `global_parameter`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `global_parameter` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(128) NOT NULL,
  `data_type` varchar(32) NOT NULL DEFAULT 'double',
  `value` text NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_global_parameter_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `global_parameter`
--

LOCK TABLES `global_parameter` WRITE;
/*!40000 ALTER TABLE `global_parameter` DISABLE KEYS */;
/*!40000 ALTER TABLE `global_parameter` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `module`
--

DROP TABLE IF EXISTS `module`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `module` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `crate_id` int(11) NOT NULL,
  `type` int(11) NOT NULL,
  `serial` varchar(128) NOT NULL,
  `slot` int(11) NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_module_serial` (`serial`),
  KEY `ix_module_crate` (`crate_id`),
  CONSTRAINT `fk_module_crate` FOREIGN KEY (`crate_id`) REFERENCES `crate` (`id`) ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=109 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `module`
--

LOCK TABLES `module` WRITE;
/*!40000 ALTER TABLE `module` DISABLE KEYS */;
INSERT INTO `module` VALUES
(1,1,11,'LTR11-000001',0,'LTR11 test module');
/*!40000 ALTER TABLE `module` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sensor`
--

DROP TABLE IF EXISTS `sensor`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `sensor` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `sensor_type_id` int(11) NOT NULL,
  `name` varchar(128) NOT NULL,
  `serial` varchar(128) NOT NULL DEFAULT '',
  `description` varchar(255) NOT NULL DEFAULT '',
  `calibration_mode` int(11) NOT NULL DEFAULT 0,
  `calibration_kind` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `ix_sensor_type` (`sensor_type_id`),
  CONSTRAINT `fk_sensor_type` FOREIGN KEY (`sensor_type_id`) REFERENCES `sensor_type` (`id`) ON UPDATE CASCADE,
  CONSTRAINT `chk_sensor_calibration_mode` CHECK (`calibration_mode` in (0,1)),
  CONSTRAINT `chk_sensor_calibration_kind` CHECK (`calibration_kind` in (0,1))
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sensor`
--

LOCK TABLES `sensor` WRITE;
/*!40000 ALTER TABLE `sensor` DISABLE KEYS */;
INSERT INTO `sensor` VALUES
(1,1,'Test sensor by type','TEST-TYPE-1','',1,0),
(2,2,'Test sensor own','TEST-OWN-2','',0,0);
/*!40000 ALTER TABLE `sensor` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sensor_type`
--

DROP TABLE IF EXISTS `sensor_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `sensor_type` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(128) NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  `calibration_kind` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_sensor_type_name` (`name`),
  CONSTRAINT `chk_sensor_type_calibration_kind` CHECK (`calibration_kind` in (0,1))
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sensor_type`
--

LOCK TABLES `sensor_type` WRITE;
/*!40000 ALTER TABLE `sensor_type` DISABLE KEYS */;
INSERT INTO `sensor_type` VALUES
(1,'Test linear sensor type','y = 0.1*x',0),
(2,'Test own-calibration sensor type','',0);
/*!40000 ALTER TABLE `sensor_type` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `signal_type`
--

DROP TABLE IF EXISTS `signal_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `signal_type` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `description` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_signal_type_name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `signal_type`
--

LOCK TABLES `signal_type` WRITE;
/*!40000 ALTER TABLE `signal_type` DISABLE KEYS */;
INSERT INTO `signal_type` VALUES
(1,'Давление',''),
(2,'Температура','');
/*!40000 ALTER TABLE `signal_type` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping routines for database 'dataserver_test'
--
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*M!100616 SET NOTE_VERBOSITY=@OLD_NOTE_VERBOSITY */;

-- Dump completed on 2026-10-21 10:45:20
