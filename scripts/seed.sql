-- DataServer deterministic integration-test fixture.
-- WARNING: replaces ALL ROWS in dataserver_test (schema is untouched).
-- Run with MariaDB client or DBeaver script execution (supports DELIMITER).
USE `dataserver_test`;

DELIMITER $$
CREATE PROCEDURE `__assert_test_database_seed`()
BEGIN
  IF DATABASE() <> 'dataserver_test' OR DATABASE() IS NULL THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Refusing to seed a non-test database';
  END IF;
END$$
DELIMITER ;
CALL `__assert_test_database_seed`();
DROP PROCEDURE `__assert_test_database_seed`;

START TRANSACTION;

-- Remove child rows before parent rows; keep foreign-key checks enabled.
DELETE FROM `calibration_coefficient`;
DELETE FROM `calibration_point`;
DELETE FROM `calibration`;
DELETE FROM `configuration_signal_definition`;
DELETE FROM `configuration_parameter`;
DELETE FROM `configuration_tag`;
DELETE FROM `configuration_module`;
DELETE FROM `configuration`;
DELETE FROM `sensor`;
DELETE FROM `sensor_type`;
DELETE FROM `signal_type`;
DELETE FROM `module`;
DELETE FROM `crate`;
DELETE FROM `global_parameter`;

INSERT INTO `crate` (`id`,`type`,`serial`,`host`,`port`,`description`) VALUES
 (1,1,'3T778029','127.0.0.1',11111,'CRATE16 test');

INSERT INTO `module` (`id`,`crate_id`,`type`,`serial`,`slot`,`description`) VALUES
 (1,1,11,'LTR11-000001',0,'LTR11 test module');

INSERT INTO `signal_type` (`id`,`name`,`description`) VALUES
 (1,'Давление',''), (2,'Температура','');

INSERT INTO `sensor_type` (`id`,`name`,`description`,`calibration_kind`) VALUES
 (1,'Test linear sensor type','y = 0.1*x',0),
 (2,'Test own-calibration sensor type','',0);

-- 0=Own, 1=ByType; 0=Table, 1=Polynomial.
INSERT INTO `sensor`
 (`id`,`sensor_type_id`,`name`,`serial`,`description`,`calibration_mode`,`calibration_kind`) VALUES
 (1,1,'Test sensor by type','TEST-TYPE-1','',1,0),
 (2,2,'Test sensor own','TEST-OWN-2','',0,0);

-- Exactly one sensor-owned and one sensor-type-owned calibration.
INSERT INTO `calibration`
 (`id`,`sensor_id`,`sensor_type_id`,`kind`,`description`) VALUES
 (1,NULL,1,0,'A: y = 0.1*x'),
 (2,2,NULL,0,'B: y = x - 20');

INSERT INTO `calibration_point` (`calibration_id`,`index`,`x`,`y`) VALUES
 (1,0,0,0), (1,1,100,10),
 (2,0,0,-20), (2,1,100,80);

INSERT INTO `configuration` (`id`,`name`,`description`,`udp_port`) VALUES
 (1,'Test experiment','Deterministic DataServer integration tests',5101);

INSERT INTO `configuration_module`
 (`configuration_id`,`module_id`,`active`,`settings`) VALUES
 (1,1,1,'{"channelRate":1000,"mode":0}');

INSERT INTO `configuration_tag`
 (`id`,`configuration_id`,`module_id`,`channel`,`active`,`settings`) VALUES
 (11,1,1,0,1,'{"range":1}'),
 (20,1,1,1,1,'{"range":1}');

-- Keep signal IDs used by existing integration tests.
-- A is calibrated by sensor type; B by its own sensor; C sums calibrated A+B.
INSERT INTO `configuration_signal_definition`
 (`id`,`configuration_id`,`name`,`kind`,`tag_id`,`sensor_id`,`signal_type_id`,`formula`,`archive_rate`) VALUES
 (6, 1,'Raw0',0,11,NULL,1,NULL,1000),
 (10,1,'Raw1',0,20,NULL,2,NULL,100),
 (3, 1,'A',1,NULL,1,1,'Raw0',100),
 (4, 1,'B',1,NULL,2,2,'Raw1',10),
 (5, 1,'C',1,NULL,NULL,2,'A + B',10);

COMMIT;

-- Expected: 1 configuration, 1 module, 2 active tags, 5 signals,
-- 2 sensors, 2 sensor types, 2 calibrations (1 per ownership category).
SELECT 'configuration' AS entity, COUNT(*) AS records FROM `configuration`
UNION ALL SELECT 'configuration_tag', COUNT(*) FROM `configuration_tag`
UNION ALL SELECT 'configuration_signal_definition', COUNT(*) FROM `configuration_signal_definition`
UNION ALL SELECT 'sensor', COUNT(*) FROM `sensor`
UNION ALL SELECT 'sensor_type', COUNT(*) FROM `sensor_type`
UNION ALL SELECT 'calibration', COUNT(*) FROM `calibration`;
