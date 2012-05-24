module YAML
  module LibYAML
    module Version
      MAJOR = 0
      MINOR = 0
      TINY  = 1

      class << self
        def to_version
          [ MAJOR, MINOR, TINY ].join('.')
        end
        def to_name
          [ MAJOR, MINOR, TINY ].join('_')
        end
      end
    end
  end
end
