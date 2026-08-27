use dataserver;

-- ============================================================
-- Crate
-- ============================================================

INSERT INTO crate
    (type, serial, host, port, description)
VALUES
    (1, '3T778029', '127.0.0.1', 11111, 'CRATE16');


-- ============================================================
-- Module
-- ============================================================

INSERT INTO module
    (crate_id, type, serial, description)
VALUES
(
    (SELECT id
     FROM crate
     WHERE serial = '3T778029'),

    11, -- LTR11

    'LTR11-000001',
    'LTR11 test module'
);


-- ============================================================
-- Configuration
-- ============================================================

INSERT INTO configuration
    (name, description, udp_port)
VALUES
    ('Иследование 1', 'Тестовая конфигурация', 5001);


-- ============================================================
-- Configuration module
-- ============================================================

INSERT INTO configuration_module
    (configuration_id, module_id, settings)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    (SELECT id
     FROM module
     WHERE serial = 'LTR11-000001'),

    '{}'
);


-- ============================================================
-- Tags
-- ============================================================

INSERT INTO configuration_tag
    (configuration_id, module_id, channel)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    (SELECT id
     FROM module
     WHERE serial = 'LTR11-000001'),

    0
);

INSERT INTO configuration_tag
    (configuration_id, module_id, channel)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    (SELECT id
     FROM module
     WHERE serial = 'LTR11-000001'),

    1
);


-- ============================================================
-- Signal types
-- ============================================================

INSERT INTO signal_type
    (name, description)
VALUES
    ('Давление', '');

INSERT INTO signal_type
    (name, description)
VALUES
    ('Температура', '');


-- ============================================================
-- Signal definitions
-- ============================================================

INSERT INTO configuration_signal_definition
(
    configuration_id,
    name,
    kind,
    tag_id,
    signal_type_id,
    archive_frequency,
    calibration_mode,
    formula
)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    'Raw0',
    0,

    (SELECT id
     FROM configuration_tag
     WHERE configuration_id =
         (SELECT id
          FROM configuration
          WHERE name = 'Иследование 1')
       AND channel = 0),

    (SELECT id
     FROM signal_type
     WHERE name = 'Давление'),

    1000,
    0,
    NULL
);


INSERT INTO configuration_signal_definition
(
    configuration_id,
    name,
    kind,
    tag_id,
    signal_type_id,
    archive_frequency,
    calibration_mode,
    formula
)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    'Raw1',
    0,

    (SELECT id
     FROM configuration_tag
     WHERE configuration_id =
         (SELECT id
          FROM configuration
          WHERE name = 'Иследование 1')
       AND channel = 1),

    (SELECT id
     FROM signal_type
     WHERE name = 'Температура'),

    100,
    0,
    NULL
);


INSERT INTO configuration_signal_definition
(
    configuration_id,
    name,
    kind,
    tag_id,
    signal_type_id,
    archive_frequency,
    calibration_mode,
    formula
)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    'A',
    1,
    NULL,

    (SELECT id
     FROM signal_type
     WHERE name = 'Давление'),

    100,
    1,
    'Raw0'
);


INSERT INTO configuration_signal_definition
(
    configuration_id,
    name,
    kind,
    tag_id,
    signal_type_id,
    archive_frequency,
    calibration_mode,
    formula
)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    'B',
    1,
    NULL,

    (SELECT id
     FROM signal_type
     WHERE name = 'Температура'),

    10,
    2,
    'Raw1'
);


INSERT INTO configuration_signal_definition
(
    configuration_id,
    name,
    kind,
    tag_id,
    signal_type_id,
    archive_frequency,
    calibration_mode,
    formula
)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    'C',
    1,
    NULL,

    (SELECT id
     FROM signal_type
     WHERE name = 'Температура'),

    10,
    0,
    'A + B'
);


-- ============================================================
-- Calibrations
-- ============================================================

INSERT INTO calibration
(
    configuration_id,
    signal_id,
    signal_type_id,
    name,
    description
)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    (SELECT id
     FROM configuration_signal_definition
     WHERE configuration_id =
         (SELECT id
          FROM configuration
          WHERE name = 'Иследование 1')
       AND name = 'A'),

    NULL,

    'Raw0 calibration',
    'Calibration for Raw0'
);


INSERT INTO calibration
(
    configuration_id,
    signal_id,
    signal_type_id,
    name,
    description
)
VALUES
(
    (SELECT id
     FROM configuration
     WHERE name = 'Иследование 1'),

    NULL,

    (SELECT id
     FROM signal_type
     WHERE name = 'Температура'),

    'Temperature calibration',
    'Calibration for temperature signals'
);


-- ============================================================
-- Calibration points
-- ============================================================

INSERT INTO calibration_point
    (calibration_id, `index`, x, y)
VALUES
(
    (SELECT id
     FROM calibration
     WHERE name = 'Raw0 calibration'),

    0, 0.0, 0.0
),
(
    (SELECT id
     FROM calibration
     WHERE name = 'Raw0 calibration'),

    1, 10.0, 1.0
),
(
    (SELECT id
     FROM calibration
     WHERE name = 'Raw0 calibration'),

    2, 20.0, 2.0
);


INSERT INTO calibration_point
    (calibration_id, `index`, x, y)
VALUES
(
    (SELECT id
     FROM calibration
     WHERE name = 'Temperature calibration'),

    0, 0.0, -20.0
),
(
    (SELECT id
     FROM calibration
     WHERE name = 'Temperature calibration'),

    1, 10.0, -10.0
),
(
    (SELECT id
     FROM calibration
     WHERE name = 'Temperature calibration'),

    2, 20.0, 0.0
);