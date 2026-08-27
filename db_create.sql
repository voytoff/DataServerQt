CREATE DATABASE IF NOT EXISTS dataserver
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE dataserver;


-- ============================================================
-- 1. Физические устройства
-- ============================================================

CREATE TABLE crate
(
    id          INT NOT NULL AUTO_INCREMENT,
    type        INT NOT NULL,
    serial      VARCHAR(128) NOT NULL,
    host        VARCHAR(255) NOT NULL,
    port        INT NOT NULL,
    description VARCHAR(255) NOT NULL DEFAULT '',

    PRIMARY KEY (id),

    UNIQUE KEY uq_crate_serial (serial)
)
ENGINE = InnoDB;


CREATE TABLE module
(
    id          INT NOT NULL AUTO_INCREMENT,
    crate_id    INT NOT NULL,
    type        INT NOT NULL,
    serial      VARCHAR(128) NOT NULL,
    description VARCHAR(255) NOT NULL DEFAULT '',

    PRIMARY KEY (id),

    UNIQUE KEY uq_module_serial (serial),
    KEY ix_module_crate (crate_id),

    CONSTRAINT fk_module_crate
        FOREIGN KEY (crate_id)
        REFERENCES crate (id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
)
ENGINE = InnoDB;


-- ============================================================
-- 2. Конфигурации
-- ============================================================

CREATE TABLE configuration
(
    id          INT NOT NULL AUTO_INCREMENT,
    name        VARCHAR(128) NOT NULL,
    description VARCHAR(255) NOT NULL DEFAULT '',
    udp_port    INT NOT NULL,

    PRIMARY KEY (id),

    UNIQUE KEY uq_configuration_name (name)
)
ENGINE = InnoDB;


-- ============================================================
-- 3. Модули конкретной configuration
-- ============================================================

CREATE TABLE configuration_module
(
    configuration_id INT NOT NULL,
    module_id        INT NOT NULL,
    settings         JSON CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,

    PRIMARY KEY (configuration_id, module_id),

    CONSTRAINT fk_configuration_module_configuration
        FOREIGN KEY (configuration_id)
        REFERENCES configuration (id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,

    CONSTRAINT fk_configuration_module_module
        FOREIGN KEY (module_id)
        REFERENCES module (id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
)
ENGINE = InnoDB;


-- ============================================================
-- 4. Tags / физические каналы configuration
--
-- TagId == configuration_tag.id
-- ============================================================

CREATE TABLE configuration_tag
(
    id               INT NOT NULL AUTO_INCREMENT,
    configuration_id INT NOT NULL,

    module_id        INT NOT NULL,
    channel          INT NOT NULL,

    PRIMARY KEY (id),

    UNIQUE KEY uq_configuration_tag_channel
        (configuration_id, module_id, channel),

    KEY ix_configuration_tag_configuration
        (configuration_id),

    KEY ix_configuration_tag_module
        (module_id),

    CONSTRAINT fk_configuration_tag_configuration
        FOREIGN KEY (configuration_id)
        REFERENCES configuration (id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,

    CONSTRAINT fk_configuration_tag_module
        FOREIGN KEY (configuration_id, module_id)
        REFERENCES configuration_module
            (configuration_id, module_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
)
ENGINE = InnoDB;


-- ============================================================
-- 5. Типы сигналов
--
-- Например:
--   Temperature
--   Pressure
--   Voltage
--   Current
-- ============================================================

CREATE TABLE signal_type
(
    id          INT NOT NULL AUTO_INCREMENT,
    name        VARCHAR(64) NOT NULL,
    description VARCHAR(255) NOT NULL DEFAULT '',

    PRIMARY KEY (id),

    UNIQUE KEY uq_signal_type_name (name)
)
ENGINE = InnoDB;


-- ============================================================
-- 6. Определения сигналов
--
-- SignalId == configuration_signal_definition.id
-- ============================================================

CREATE TABLE configuration_signal_definition
(
    id                INT NOT NULL AUTO_INCREMENT,
    configuration_id  INT NOT NULL,

    name              VARCHAR(128) NOT NULL,

    -- SignalKind:
    --   0 = Raw
    --   1 = Calculated
    kind              INT NOT NULL,

    -- Для Raw.
    -- Для Calculated = NULL.
    tag_id            INT NULL,

    -- Тип датчика / физического сигнала.
    signal_type_id    INT NULL,

    -- Частота записи в архив.
    archive_frequency INT NOT NULL,

    -- CalibrationMode:
    --   0 = None
    --   1 = BySignal
    --   2 = BySignalType
    calibration_mode  INT NOT NULL DEFAULT 0,

    -- Только для Calculated.
    formula           TEXT NULL,

    PRIMARY KEY (id),

    UNIQUE KEY uq_configuration_signal_name
        (configuration_id, name),

    KEY ix_signal_configuration
        (configuration_id),

    KEY ix_signal_tag
        (tag_id),

    KEY ix_signal_type
        (signal_type_id),

    CONSTRAINT fk_signal_configuration
        FOREIGN KEY (configuration_id)
        REFERENCES configuration (id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,

    CONSTRAINT fk_signal_tag
        FOREIGN KEY (tag_id)
        REFERENCES configuration_tag (id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE,

    CONSTRAINT fk_signal_type
        FOREIGN KEY (signal_type_id)
        REFERENCES signal_type (id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE,

    CONSTRAINT chk_signal_kind
        CHECK (kind IN (0, 1)),

    CONSTRAINT chk_calibration_mode
        CHECK (calibration_mode IN (0, 1, 2)),

    CONSTRAINT chk_archive_frequency
        CHECK (archive_frequency IN (1, 10, 100, 1000))
)
ENGINE = InnoDB;


-- ============================================================
-- 7. Калибровки
--
-- CalibrationId == calibration.id
--
-- Либо:
--   signal_id
--
-- либо:
--   signal_type_id
-- ============================================================

CREATE TABLE calibration
(
    id               INT NOT NULL AUTO_INCREMENT,

    configuration_id INT NOT NULL,

    signal_id        INT NULL,
    signal_type_id   INT NULL,

    name             VARCHAR(128) NOT NULL DEFAULT '',
    description      VARCHAR(255) NOT NULL DEFAULT '',

    PRIMARY KEY (id),

    KEY ix_calibration_configuration
        (configuration_id),

    KEY ix_calibration_signal
        (signal_id),

    KEY ix_calibration_signal_type
        (signal_type_id),

    CONSTRAINT fk_calibration_configuration
        FOREIGN KEY (configuration_id)
        REFERENCES configuration (id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,

    CONSTRAINT fk_calibration_signal
        FOREIGN KEY (signal_id)
        REFERENCES configuration_signal_definition (id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,

    CONSTRAINT fk_calibration_signal_type
        FOREIGN KEY (signal_type_id)
        REFERENCES signal_type (id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE
)
ENGINE = InnoDB;


-- ============================================================
-- 8. Точки калибровки
-- ============================================================

CREATE TABLE calibration_point
(
    calibration_id INT NOT NULL,
    `index`        INT NOT NULL,

    x              DOUBLE NOT NULL,
    y              DOUBLE NOT NULL,

    PRIMARY KEY (calibration_id, `index`),

    CONSTRAINT fk_calibration_point_calibration
        FOREIGN KEY (calibration_id)
        REFERENCES calibration (id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
)
ENGINE = InnoDB;


-- ============================================================
-- 9. Глобальные параметры
--
-- GlobalParameterId == global_parameter.id
-- ============================================================

CREATE TABLE global_parameter
(
    id          INT NOT NULL AUTO_INCREMENT,

    name        VARCHAR(128) NOT NULL,
    data_type   VARCHAR(32) NOT NULL DEFAULT 'double',
    value       TEXT NOT NULL,
    description VARCHAR(255) NOT NULL DEFAULT '',

    PRIMARY KEY (id),

    UNIQUE KEY uq_global_parameter_name (name)
)
ENGINE = InnoDB;


-- ============================================================
-- 10. Параметры configuration
--
-- Имя уникально внутри configuration.
-- Имеет приоритет над global_parameter.
-- ============================================================

CREATE TABLE configuration_parameter
(
    id               INT NOT NULL AUTO_INCREMENT,

    configuration_id INT NOT NULL,

    name             VARCHAR(128) NOT NULL,
    data_type        VARCHAR(32) NOT NULL DEFAULT 'double',
    value            TEXT NOT NULL,
    description      VARCHAR(255) NOT NULL DEFAULT '',

    PRIMARY KEY (id),

    UNIQUE KEY uq_configuration_parameter_name
        (configuration_id, name),

    KEY ix_configuration_parameter_configuration
        (configuration_id),

    CONSTRAINT fk_configuration_parameter_configuration
        FOREIGN KEY (configuration_id)
        REFERENCES configuration (id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
)
ENGINE = InnoDB;